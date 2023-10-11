// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "../public/RangedWeaponBase.h"
#include "ShootWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API AShootWeaponBase : public ARangedWeaponBase
{
	GENERATED_BODY()

public:
	AShootWeaponBase();

	//공격 처리
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//공격 마무리 처리
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual float GetAttackRange() override;

	UFUNCTION(BlueprintCallable)
		virtual bool TryFireProjectile() override;

	//장전을 시도. 현재 상태에 따른 성공 여부를 반환
	UFUNCTION(BlueprintCallable)
		virtual bool TryReload() override;

	//탄환의 개수를 더함 (ExtraAmmoCount까지)
	UFUNCTION(BlueprintCallable)
		virtual void AddAmmo(int32 Count) override;

//------ Shoot 관련 ----------------------------------
public:
	//탄창의 개수를 더함 (+= MaxAmmoPerMagazine * Count)
	UFUNCTION(BlueprintCallable)
		void AddMagazine(int32 Count);
};
