// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/RangedWeaponBase.h"
#include "../public/ProjectileBase.h"
#include "../public/WeaponBase.h"
#include "../../Global/public/DebuffManager.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#define AUTO_RELOAD_DELAY_VALUE 0.1

void ARangedWeaponBase::Attack()
{
	Super::Attack();

	this->Tags.Add("Ranged");

	if (WeaponStat.RangedWeaponStat.bHasProjectile)
	{
		bool result = TryFireProjectile();
		PlayEffectSound(result ? AttackSound : AttackFailSound);
		if (result)	UpdateDurabilityState();
	}
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
			&& WeaponState.RangedWeaponState.TotalAmmoCount == 0.0f))
	{
		return 150.0f; //매크로나 const 멤버로 수정하기 
	}
	return 700.0f;
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
	AProjectileBase* newProjectile = Cast<AProjectileBase>(GetWorld()->SpawnActor(ProjectileClass, &SpawnTransform, spawmParams));

	if (IsValid(newProjectile))
	{
		newProjectile->SetOwner(this);
		newProjectile->InitProjectile(OwnerCharacterRef.Get());
		newProjectile->ProjectileStat.ProjectileDamage *= WeaponStat.WeaponDamageRate; //버프/디버프로 인해 강화/너프된 값을 반영
		newProjectile->ProjectileStat.ProjectileSpeed *= WeaponStat.WeaponAtkSpeedRate;
		return newProjectile;
	}
	return nullptr;
}	
	
bool ARangedWeaponBase::TryReload()
{
	if (!GetCanReload()) return false;

	int32 addAmmoCnt = FMath::Min(WeaponState.RangedWeaponState.TotalAmmoCount, WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine);
	if (addAmmoCnt + WeaponState.RangedWeaponState.CurrentAmmoCount > WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine)
	{
		addAmmoCnt = (WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine - WeaponState.RangedWeaponState.CurrentAmmoCount);
	}
	WeaponState.RangedWeaponState.CurrentAmmoCount += addAmmoCnt;
	WeaponState.RangedWeaponState.TotalAmmoCount -= addAmmoCnt;

	return true;
}	
	
bool ARangedWeaponBase::GetCanReload()
{	
	if (WeaponStat.RangedWeaponStat.bIsInfiniteAmmo || !WeaponStat.RangedWeaponStat.bHasProjectile) return false;
	if (WeaponState.RangedWeaponState.TotalAmmoCount == 0 || WeaponState.RangedWeaponState.CurrentAmmoCount == WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine) return false;
	return true;
}

void ARangedWeaponBase::AddAmmo(int32 Count)
{
	if (WeaponStat.RangedWeaponStat.bIsInfiniteAmmo || WeaponState.RangedWeaponState.TotalAmmoCount >= MAX_AMMO_LIMIT_CNT) return;
	WeaponState.RangedWeaponState.TotalAmmoCount = (WeaponState.RangedWeaponState.TotalAmmoCount + Count) % MAX_AMMO_LIMIT_CNT;
}

void ARangedWeaponBase::AddMagazine(int32 Count)
{
	if (WeaponStat.RangedWeaponStat.bIsInfiniteAmmo || WeaponState.RangedWeaponState.TotalAmmoCount >= MAX_AMMO_LIMIT_CNT) return;
	WeaponState.RangedWeaponState.TotalAmmoCount = (WeaponState.RangedWeaponState.TotalAmmoCount + WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine * Count) % MAX_AMMO_LIMIT_CNT;
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
			UE_LOG(LogTemp, Warning, TEXT("No Ammo"));
		}

		//StopAttack의 ResetActionState로 인해 실행이 되지 않는 현상 방지를 위해
		//타이머를 통해 일정 시간이 지난 후에 Reload를 시도.
		GetWorldTimerManager().SetTimer(AutoReloadTimerHandle, FTimerDelegate::CreateLambda([&]() {
			OwnerCharacterRef->TryReload();
		}), 1.0f, false, AUTO_RELOAD_DELAY_VALUE);
		return false;
	}

	const int32 fireProjectileCnt = FMath::Min(WeaponState.RangedWeaponState.CurrentAmmoCount, OwnerCharacterRef->GetCharacterStat().MaxProjectileCount);
	for (int idx = 1; idx <= fireProjectileCnt; idx++)
	{
		FTimerHandle delayHandle;
		GetWorld()->GetTimerManager().SetTimer(delayHandle, FTimerDelegate::CreateLambda([&]() {
			AProjectileBase* newProjectile = CreateProjectile();
		//스폰한 발사체가 Valid 하다면 발사
		if (IsValid(newProjectile))
		{
			if (!WeaponStat.RangedWeaponStat.bIsInfiniteAmmo && !OwnerCharacterRef->GetCharacterStat().bInfinite)
			{
				WeaponState.RangedWeaponState.CurrentAmmoCount -= 1;
			}
			newProjectile->ActivateProjectileMovement();
		}
			}), 0.1f * (float)idx, false);
	}
	return true;
}
