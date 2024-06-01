// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootWeaponBase.h"
#include "../Throw/ProjectileBase.h"
#include "../WeaponBase.h"
#include "../WeaponInventoryBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Character/CharacterBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#define AUTO_RELOAD_DELAY_VALUE 0.1
	
AShootWeaponBase::AShootWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	this->Tags.Add("Shoot");
}

void AShootWeaponBase::Attack()
{
	Super::Attack();
}

void AShootWeaponBase::StopAttack()
{
	Super::StopAttack();
}

float AShootWeaponBase::GetAttackRange()
{
	return 1000.0f;
}

bool AShootWeaponBase::TryFireProjectile()
{
	return Super::TryFireProjectile();
}

void AShootWeaponBase::Reload()
{
	Super::Reload();
}

void AShootWeaponBase::AddAmmo(int32 Count)
{
	Super::AddAmmo(Count);
}

void AShootWeaponBase::AddMagazine(int32 Count)
{
	if (WeaponStat.RangedWeaponStat.bIsInfiniteAmmo || WeaponState.RangedWeaponState.ExtraAmmoCount >= MAX_AMMO_LIMIT_CNT) return;
	WeaponState.RangedWeaponState.ExtraAmmoCount = (WeaponState.RangedWeaponState.ExtraAmmoCount + WeaponStat.RangedWeaponStat.MaxAmmoPerMagazine * Count) % MAX_AMMO_LIMIT_CNT;
}
