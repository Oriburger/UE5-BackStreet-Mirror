// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/RangedWeaponBase.h"
#include "../public/ProjectileBase.h"
#include "../public/WeaponBase.h"
#include "../public/WeaponInventoryBase.h"
#include "../../Global/public/DebuffManager.h"
#include "NiagaraFunctionLibrary.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#define AUTO_RELOAD_DELAY_VALUE 0.1

ARangedWeaponBase::ARangedWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	this->Tags.Add("Ranged");

}

void ARangedWeaponBase::Attack()
{
	if (WeaponID == 0) return; 
	Super::Attack();

	bool result = TryFireProjectile();
	PlayEffectSound(result ? AttackSound : AttackFailSound);
	if (result)	UpdateDurabilityState();
}

void ARangedWeaponBase::StopAttack()
{
	Super::StopAttack();
}

float ARangedWeaponBase::GetAttackRange()
{
	if (!OwnerCharacterRef.IsValid()) return 200.0f;
	if (WeaponStat.WeaponType == EWeaponType::E_Melee
		|| (!OwnerCharacterRef->GetCharacterStat().bInfinite
			&& WeaponState.RangedWeaponState.CurrentAmmoCount == 0.0f
			&& WeaponState.RangedWeaponState.ExtraAmmoCount == 0.0f))
	{
		return 150.0f; //매크로나 const 멤버로 수정하기 
	}
	return 700.0f;
}

FProjectileAssetInfoStruct ARangedWeaponBase::GetProjectileAssetInfo(int32 TargetProjectileID)
{
	//캐시에 기록이 되어있다면?
	if (ProjectileAssetInfo.ProjectileID == TargetProjectileID) return ProjectileAssetInfo;

	//없다면 새로 읽어옴
	else if (ProjectileAssetInfoTable != nullptr && TargetProjectileID != 0)
	{
		FProjectileAssetInfoStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetProjectileID);

		newInfo = ProjectileAssetInfoTable->FindRow<FProjectileAssetInfoStruct>(FName(rowName), rowName);
		if (newInfo != nullptr) return *newInfo;
	}
	return FProjectileAssetInfoStruct();
}

void ARangedWeaponBase::ClearAllTimerHandle()
{
	Super::ClearAllTimerHandle();
	GetWorldTimerManager().ClearTimer(AutoReloadTimerHandle);
} 

void ARangedWeaponBase::UpdateWeaponStat(FWeaponStatStruct NewStat)
{
	//WeaponStat 업데이트
}

void ARangedWeaponBase::InitWeaponAsset()
{
	Super::InitWeaponAsset();

	FRangedWeaponAssetInfoStruct& rangedWeaponAssetInfo = WeaponAssetInfo.RangedWeaponAssetInfo;

	if (rangedWeaponAssetInfo.ShootEffectParticle.IsValid())
	{
		ShootNiagaraEmitter = rangedWeaponAssetInfo.ShootEffectParticle.Get();
	}
	if (rangedWeaponAssetInfo.ProjectileID && ProjectileAssetInfoTable != nullptr)
	{
		ProjectileAssetInfo = GetProjectileAssetInfo(rangedWeaponAssetInfo.ProjectileID);
	}
}

void ARangedWeaponBase::SpawnShootNiagaraEffect()
{
	if (!OwnerCharacterRef.IsValid()) return;

	FVector spawnLocation = OwnerCharacterRef.Get()->GetActorLocation() + OwnerCharacterRef.Get()->GetMesh()->GetRightVector() * 50.0f;
	FRotator spawnRotation = OwnerCharacterRef.Get()->GetMesh()->GetComponentRotation() + FRotator(0.0f, 90.0f, 0.0f);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShootNiagaraEmitter, spawnLocation, spawnRotation);
}

AProjectileBase* ARangedWeaponBase::CreateProjectile()
{
	FActorSpawnParameters spawmParams;
	spawmParams.Owner = this; //Projectile의 소유자는 Player
	spawmParams.Instigator = GetInstigator();
	spawmParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = OwnerCharacterRef->GetActorLocation();
	FRotator SpawnRotation = OwnerCharacterRef->GetMesh()->GetComponentRotation();

	SpawnLocation = SpawnLocation + OwnerCharacterRef->GetMesh()->GetForwardVector() * 20.0f;
	SpawnLocation = SpawnLocation + OwnerCharacterRef->GetMesh()->GetRightVector() * 50.0f;

	SpawnRotation.Pitch = SpawnRotation.Roll = 0.0f;
	SpawnRotation.Yaw += 90.0f;

	FTransform SpawnTransform = { SpawnRotation, SpawnLocation, {1.0f, 1.0f, 1.0f} };
	AProjectileBase* newProjectile = Cast<AProjectileBase>(GetWorld()->SpawnActor(AProjectileBase::StaticClass(), &SpawnTransform, spawmParams));

	if (IsValid(newProjectile) && ProjectileAssetInfo.ProjectileID != 0)
	{
		newProjectile->SetOwner(this);
		newProjectile->InitProjectile(OwnerCharacterRef.Get(), ProjectileAssetInfo);
		newProjectile->ProjectileStat.ProjectileDamage *= WeaponStat.WeaponDamageRate; //버프/디버프로 인해 강화/너프된 값을 반영
		newProjectile->ProjectileStat.ProjectileSpeed *= WeaponStat.WeaponAtkSpeedRate;
		return newProjectile;
	}
	return nullptr;
}	
	
bool ARangedWeaponBase::TryReload()
{
	if (!GetCanReload()) return false;

	int32 addAmmoCnt = FMath::Min(WeaponState.RangedWeaponState.ExtraAmmoCount, WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine);
	if (addAmmoCnt + WeaponState.RangedWeaponState.CurrentAmmoCount > WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine)
	{
		addAmmoCnt = (WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine - WeaponState.RangedWeaponState.CurrentAmmoCount);
	}
	WeaponState.RangedWeaponState.CurrentAmmoCount += addAmmoCnt;
	WeaponState.RangedWeaponState.ExtraAmmoCount -= addAmmoCnt;

	return true;
}	
	
bool ARangedWeaponBase::GetCanReload()
{	
	if (WeaponStat.RangedWeaponStat.bIsInfiniteAmmo) return false;
	if (GetLeftAmmoCount() == 0) return false;
	return true;
}

void ARangedWeaponBase::AddAmmo(int32 Count)
{
	if (WeaponStat.RangedWeaponStat.bIsInfiniteAmmo || WeaponState.RangedWeaponState.ExtraAmmoCount >= MAX_AMMO_LIMIT_CNT) return;
	WeaponState.RangedWeaponState.ExtraAmmoCount = (WeaponState.RangedWeaponState.ExtraAmmoCount + Count) % MAX_AMMO_LIMIT_CNT;
}

bool ARangedWeaponBase::TryFireProjectile()
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (!WeaponStat.RangedWeaponStat.bIsInfiniteAmmo 
		&& !OwnerCharacterRef->GetCharacterStat().bInfinite && WeaponState.RangedWeaponState.CurrentAmmoCount == 0)
	{
		if (OwnerCharacterRef->ActorHasTag("Player"))
		{
			GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("탄환이 부족합니다.")), FColor::White);
		}

		//StopAttack의 ResetActionState로 인해 실행이 되지 않는 현상 방지를 위해
		//타이머를 통해 일정 시간이 지난 후에 Reload를 시도.
		GetWorldTimerManager().SetTimer(AutoReloadTimerHandle, FTimerDelegate::CreateLambda([&]() {
			OwnerCharacterRef->TryReload();
		}), 1.0f, false, AUTO_RELOAD_DELAY_VALUE);
		return false;
	}
	const int32 fireProjectileCnt = FMath::Min(WeaponState.RangedWeaponState.CurrentAmmoCount, OwnerCharacterRef.Get()->GetCharacterStat().MaxProjectileCount);
	for (int idx = 1; idx <= fireProjectileCnt; idx++)
	{
		FTimerHandle delayHandle;

		GetWorld()->GetTimerManager().SetTimer(delayHandle, FTimerDelegate::CreateLambda([&]() {
			AProjectileBase* newProjectile = CreateProjectile();
			//스폰한 발사체가 Valid 하다면 발사
			if (IsValid(newProjectile))
			{
				if (!WeaponStat.RangedWeaponStat.bIsInfiniteAmmo && !OwnerCharacterRef.Get()->GetCharacterStat().bInfinite)
				{
					WeaponState.RangedWeaponState.CurrentAmmoCount -= 1;
					OwnerCharacterRef.Get()->GetInventoryRef()->SyncCurrentWeaponInfo(true);
				}
				newProjectile->ActivateProjectileMovement();
				SpawnShootNiagaraEffect(); //발사와 동시에 이미터를 출력한다.
			}

			//탄환이 다 되었다면? 자동으로 제거
			if (WeaponState.RangedWeaponState.CurrentAmmoCount == 0 && WeaponState.RangedWeaponState.ExtraAmmoCount == 0)
			{
				//플레이어가 소유한 보조무기라면? 보조무기 인벤토리에서 제거
				if (OwnerCharacterRef.Get()->ActorHasTag("Player") && WeaponStat.WeaponType == EWeaponType::E_Throw)
				{
					Cast<AMainCharacterBase>(OwnerCharacterRef.Get())->GetSubInventoryRef()->RemoveCurrentWeapon();
				}
				//그렇지 않다면 일반 인벤토리에서 제거 
				else
				{
					OwnerCharacterRef.Get()->GetInventoryRef()->RemoveCurrentWeapon();
				}
			}

		}), 0.1f * (float)idx, false);
	}
	
	return true;
}
