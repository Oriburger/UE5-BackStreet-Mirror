// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedCombatManager.h"
#include "../CombatManager.h"
#include "../Throw/ProjectileBase.h"
#include "../../../System/AssetSystem/AssetManagerBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Character/MainCharacter/MainCharacterBase.h"
#include "../../../Character/Component/WeaponComponentBase.h"
#include "NiagaraFunctionLibrary.h"
#define AUTO_RELOAD_DELAY_VALUE 0.1

URangedCombatManager::URangedCombatManager()
{
}

void URangedCombatManager::Attack()
{
	Super::Attack();

	bool result = TryFireProjectile();

	if (AssetManagerRef.IsValid())
	{
		AssetManagerRef.Get()->PlaySingleSound(OwnerCharacterRef.Get(), ESoundAssetType::E_Weapon
		, WeaponComponentRef.Get()->WeaponID, result ? "Shoot" : "ShootFail");
	}
}

void URangedCombatManager::StopAttack()
{
	Super::Attack();
}

bool URangedCombatManager::TryFireProjectile(FRotator FireRotationOverride)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (!WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo
		&& !OwnerCharacterRef->GetCharacterStat().bInfinite && WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount == 0)
	{
		if (OwnerCharacterRef->ActorHasTag("Player"))
		{
			GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("źȯ�� �����մϴ�.")), FColor::White);
		}

		//StopAttack�� ResetActionState�� ���� ������ ���� �ʴ� ���� ������ ����
		//Ÿ�̸Ӹ� ���� ���� �ð��� ���� �Ŀ� Reload�� �õ�.
		GamemodeRef.Get()->GetWorldTimerManager().SetTimer(AutoReloadTimerHandle, OwnerCharacterRef.Get(), &ACharacterBase::TryReload, 1.0f, false, AUTO_RELOAD_DELAY_VALUE);
		return false;
	}

	int32 fireProjectileCnt = OwnerCharacterRef.Get()->GetCharacterStat().ProjectileCountPerAttack;

	if (!WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo 
		&& !OwnerCharacterRef.Get()->GetCharacterStat().bInfinite)
	{
		fireProjectileCnt = FMath::Min(fireProjectileCnt, WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount);
		WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount -= 1;
	}

	FireRotationForTimer = FireRotationOverride;

	for (int idx = 1; idx <= fireProjectileCnt; idx++)
	{
		FTimerHandle delayHandle;
		GetWorld()->GetTimerManager().SetTimer(delayHandle, FTimerDelegate::CreateLambda([&]() {
			AProjectileBase* newProjectile = CreateProjectile(FireRotationForTimer);
			//������ �߻�ü�� Valid �ϴٸ� �߻�
			if (IsValid(newProjectile))
			{
				newProjectile->ActivateProjectileMovement();
				SpawnShootNiagaraEffect(); //�߻�� ���ÿ� �̹��͸� ����Ѵ�.
			}
		}), 0.1f * (float)idx, false);
	}
	return true;
}

void URangedCombatManager::Reload()
{
	if (!GetCanReload()) return;

	FWeaponStatStruct weaponStat = WeaponComponentRef.Get()->WeaponStat;
	FWeaponStateStruct weaponState = WeaponComponentRef.Get()->WeaponState;
	int32 addAmmoCnt = FMath::Min(weaponState.RangedWeaponState.ExtraAmmoCount, weaponStat.RangedWeaponStat.MaxAmmoPerMagazine);

	if (addAmmoCnt + weaponState.RangedWeaponState.CurrentAmmoCount > weaponStat.RangedWeaponStat.MaxAmmoPerMagazine)
	{
		addAmmoCnt = (weaponStat.RangedWeaponStat.MaxAmmoPerMagazine - weaponState.RangedWeaponState.CurrentAmmoCount);
	}
	weaponState.RangedWeaponState.CurrentAmmoCount += addAmmoCnt;
	weaponState.RangedWeaponState.ExtraAmmoCount -= addAmmoCnt;

	//if (OwnerCharacterRef.IsValid())
	//	OwnerCharacterRef.Get()->ResetActionState(true);

	if (AssetManagerRef.IsValid())
	{
		AssetManagerRef.Get()->PlaySingleSound(OwnerCharacterRef.Get(), ESoundAssetType::E_Weapon
								, WeaponComponentRef.Get()->WeaponID, "Reload");
	}
}

void URangedCombatManager::AddAmmo(int32 Count)
{
	if (WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo) return;
	if (WeaponComponentRef.Get()->WeaponState.RangedWeaponState.ExtraAmmoCount >= 1e5) return;
	WeaponComponentRef.Get()->WeaponState.RangedWeaponState.ExtraAmmoCount = 
		(WeaponComponentRef.Get()->WeaponState.RangedWeaponState.ExtraAmmoCount + Count) % (int32)1e5;
}

AProjectileBase* URangedCombatManager::CreateProjectile(FRotator FireRotationOverride)
{	
	FWeaponStatStruct weaponStat = WeaponComponentRef.Get()->WeaponStat;
	FWeaponStateStruct weaponState = WeaponComponentRef.Get()->WeaponState;

	FActorSpawnParameters spawmParams;
	spawmParams.Owner = OwnerCharacterRef.Get(); //Projectile�� �����ڴ� Player
	spawmParams.Instigator = OwnerCharacterRef.Get()->GetInstigator();
	spawmParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector spawnLocation = OwnerCharacterRef.Get()->WeaponComponent->GetComponentLocation();
	FRotator spawnRotation = FireRotationOverride.IsNearlyZero(0.001f) ?
								OwnerCharacterRef.Get()->GetMesh()->GetComponentRotation()
								: FireRotationOverride;


	//ĳ������ Forward �������� �����ش�.
	if (FireRotationOverride.IsNearlyZero(0.001f))
	{
		spawnRotation.Pitch = spawnRotation.Roll = 0.0f;
		spawnRotation.Yaw += 90.0f;
	}

	FTransform SpawnTransform = { spawnRotation, spawnLocation, {1.0f, 1.0f, 1.0f} };
	AProjectileBase* newProjectile = Cast<AProjectileBase>(GetWorld()->SpawnActor(AProjectileBase::StaticClass(), &SpawnTransform, spawmParams));

	if (IsValid(newProjectile) && WeaponComponentRef.Get()->ProjectileAssetInfo.ProjectileID != 0)
	{
		newProjectile->SetOwner(OwnerCharacterRef.Get());
		newProjectile->InitProjectile(OwnerCharacterRef.Get(), WeaponComponentRef.Get()->ProjectileAssetInfo, WeaponComponentRef.Get()->ProjectileStatInfo);
		newProjectile->ProjectileStat.DebuffInfo = WeaponComponentRef.Get()->WeaponStat.DebuffInfo;
		newProjectile->ProjectileStat.ProjectileDamage = WeaponComponentRef.Get()->WeaponStat.WeaponDamage;
		return newProjectile;
	}
	return nullptr;
}

void URangedCombatManager::SetInfiniteAmmoMode(bool NewMode)
{
	WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo = NewMode;
}

int32 URangedCombatManager::GetLeftAmmoCount()
{
	return WeaponComponentRef.Get()->WeaponState.RangedWeaponState.ExtraAmmoCount
			+ WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount;
}

bool URangedCombatManager::GetCanReload()
{
	if (WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo) return false;
	if (GetLeftAmmoCount() == 0) return false;
	return true;
}

void URangedCombatManager::SpawnShootNiagaraEffect()
{
	if (!OwnerCharacterRef.IsValid()) return;

	FVector spawnLocation = OwnerCharacterRef.Get()->GetActorLocation() + OwnerCharacterRef.Get()->GetMesh()->GetRightVector() * 50.0f;
	FRotator spawnRotation = OwnerCharacterRef.Get()->GetMesh()->GetComponentRotation() + FRotator(0.0f, 0.0f, 0.0f);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShootNiagaraEmitter, spawnLocation, spawnRotation);
}
