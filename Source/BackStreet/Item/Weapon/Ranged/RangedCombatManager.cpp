// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedCombatManager.h"
#include "../CombatManager.h"
#include "../Throw/ProjectileBase.h"
#include "../../../System/AssetSystem/AssetManagerBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Character/Component/ItemInventoryComponent.h"
#include "../../../Character/MainCharacter/MainCharacterBase.h"
#include "../../../Character/Component/WeaponComponentBase.h"
#include "NiagaraFunctionLibrary.h"

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
		GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("보조무기가 없습니다.")), FColor::White);
		return false;
	}

	int32 fireProjectileCnt = OwnerCharacterRef.Get()->GetCharacterStat().ProjectileCountPerAttack;

	if (!WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo 
		&& !OwnerCharacterRef.Get()->GetCharacterStat().bInfinite)
	{
		fireProjectileCnt = FMath::Min(fireProjectileCnt, WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount);
		WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount -= 1;
	}
	
	const int32 fireCount = OwnerCharacterRef.Get()->GetCharacterStat().ProjectileCountPerAttack;
	TArray<FRotator> rotationList = GetFireRotationList(fireCount);
	for (int32 idx = 0; idx < fireCount; idx++)
	{
		FRotator fireRotation = FireRotationOverride;
		if (rotationList.IsValidIndex(idx) && FireRotationOverride.IsNearlyZero(0.1f))
		{
			fireRotation = rotationList[idx];
		}
		
		AProjectileBase* newProjectile = CreateProjectile(fireRotation);
		//스폰한 발사체가 Valid 하다면 발사
		if (IsValid(newProjectile))
		{
			newProjectile->ActivateProjectileMovement();
			SpawnShootNiagaraEffect(); //발사와 동시에 이미터를 출력한다.
		}
	}

	//broadcast delegate
	const int32 weaponID = WeaponComponentRef.Get()->WeaponID;
	const EWeaponType weaponType = WeaponComponentRef.Get()->GetWeaponStat().WeaponType;
	const FWeaponStateStruct weaponState = WeaponComponentRef.Get()->GetWeaponState();
	WeaponComponentRef.Get()->OnWeaponStateUpdated.Broadcast(weaponID, weaponType, weaponState);

	//탄환을 다 썼을 경우, 모든 탄환을 발사한 이후 아래 로직을 처리한다
	if (OwnerCharacterRef.Get()->ActorHasTag("Player")
		&& WeaponComponentRef.Get()->GetWeaponState().RangedWeaponState.GetIsEmpty())
	{
		//Cast<AMainCharacterBase>(OwnerCharacterRef.Get())->ZoomOut();
		//Cast<AMainCharacterBase>(OwnerCharacterRef.Get())->SwitchWeapon(false);
	}
	return true;
}

void URangedCombatManager::AddAmmo(int32 Count)
{
	if (WeaponComponentRef.Get()->WeaponStat.RangedWeaponStat.bIsInfiniteAmmo) return;
	
	//broadcast delegate
	const int32 weaponID = WeaponComponentRef.Get()->WeaponID;
	const EWeaponType weaponType = WeaponComponentRef.Get()->GetWeaponStat().WeaponType;
	FWeaponStateStruct weaponState = WeaponComponentRef.Get()->GetWeaponState();
	weaponState.RangedWeaponState.CurrentAmmoCount += Count;
	weaponState.RangedWeaponState.UpdateAmmoValidation(WeaponComponentRef.Get()->GetWeaponStat().RangedWeaponStat.MaxTotalAmmo);

	WeaponComponentRef.Get()->OnWeaponStateUpdated.Broadcast(weaponID, weaponType, weaponState);
}

AProjectileBase* URangedCombatManager::CreateProjectile(FRotator FireRotationOverride)
{	
	FWeaponStatStruct weaponStat = WeaponComponentRef.Get()->WeaponStat;
	FWeaponStateStruct weaponState = WeaponComponentRef.Get()->WeaponState;

	FActorSpawnParameters spawmParams;
	spawmParams.Owner = OwnerCharacterRef.Get(); //Projectile의 소유자는 Player
	spawmParams.Instigator = OwnerCharacterRef.Get()->GetInstigator();
	spawmParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector spawnLocation = OwnerCharacterRef.Get()->WeaponComponent->GetComponentLocation()
							+ OwnerCharacterRef.Get()->GetMesh()->GetRightVector() * 25.0f;
	FRotator spawnRotation = FireRotationOverride.IsNearlyZero(0.01f) ?
								OwnerCharacterRef.Get()->GetMesh()->GetComponentRotation()
								: FireRotationOverride;
	
	//캐릭터의 Forward 방향으로 맞춰준다.
	if (FireRotationOverride.IsNearlyZero(0.001f))
	{
		spawnRotation.Pitch = spawnRotation.Roll = 0.0f;
		spawnRotation.Yaw += 90.0f;
	}

	FTransform spawnTransform = { spawnRotation, spawnLocation, {1.0f, 1.0f, 1.0f} };
	AProjectileBase* newProjectile = Cast<AProjectileBase>(GetWorld()->SpawnActor(AProjectileBase::StaticClass(), &spawnTransform, spawmParams));

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
	return WeaponComponentRef.Get()->WeaponState.RangedWeaponState.CurrentAmmoCount;
}

TArray<FRotator> URangedCombatManager::GetFireRotationList(int32 FireCount)
{
	//RotationList for return
	TArray<FRotator> rotationList;
	//Get Character Rotation Yaw = Standard
	float standardRotation = OwnerCharacterRef.Get()->GetActorRotation().Yaw;
	//Get min and max Angle
	float maxFireAngle = 100.0f / (FireCount + 1.0f);
	float minFireAngle = -maxFireAngle;
	float step = (maxFireAngle - minFireAngle) / (FireCount - 1.0f);
	int32 halfReps = FireCount / 2;

	//Calculation Rotation Yaw and add to rotationList
	for (int32 idx = -halfReps; idx <= halfReps; idx++)
	{
		// idx == 0 add ZeroRotator
		if (idx == 0)
		{
			rotationList.Add(FRotator::ZeroRotator);
			continue;
		}

		//Calculation Yaw  
		float resultYaw = static_cast<float>(idx) * step / halfReps;
		rotationList.Add(FRotator(0.0f,standardRotation + resultYaw, 0.0f));
	}

	return rotationList;
}

void URangedCombatManager::SpawnShootNiagaraEffect()
{
	if (!OwnerCharacterRef.IsValid()) return;

	FVector spawnLocation = OwnerCharacterRef.Get()->GetActorLocation() + OwnerCharacterRef.Get()->GetMesh()->GetRightVector() * 50.0f;
	FRotator spawnRotation = OwnerCharacterRef.Get()->GetMesh()->GetComponentRotation() + FRotator(0.0f, 0.0f, 0.0f);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShootNiagaraEmitter, spawnLocation, spawnRotation);
}
