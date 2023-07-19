// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "../public/WeaponBase.h"
#include "RangedWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API ARangedWeaponBase : public AWeaponBase
{
	GENERATED_BODY()

public:
	//공격 처리
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//공격 마무리 처리
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual float GetAttackRange() override;

//------ 스탯/상태 관련 ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		virtual void UpdateWeaponStat(FWeaponStatStruct NewStat) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FRangedWeaponStateStruct RangedWeaponState;

//------ Projectile 관련----------------------------
public:
	//발사체를 생성
	UFUNCTION()
		class AProjectileBase* CreateProjectile();

	//발사체 초기화 및 발사를 시도 
	UFUNCTION()
		bool TryFireProjectile();

	//장전을 시도. 현재 상태에 따른 성공 여부를 반환
	UFUNCTION(BlueprintCallable)
		bool TryReload();

	//남은 탄환의 개수를 반환 - Stat.TotalAmmoCount
	UFUNCTION(BlueprintCallable)
		int32 GetLeftAmmoCount() { return RangedWeaponState.TotalAmmoCount; };

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetCanReload();

	//탄환의 개수를 더함 (TotalAmmoCount까지)
	UFUNCTION(BlueprintCallable)
		void AddAmmo(int32 Count);

	//탄창의 개수를 더함 (+= MaxAmmoPerMagazine*Count)
	UFUNCTION()
		void AddMagazine(int32 Count);

	UFUNCTION()
		void SetInfiniteAmmoMode(bool NewMode) { WeaponStat.RangedWeaponStat.bIsInfiniteAmmo = NewMode; }

protected:
	//SoftObjRef로 대체 예정
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Weapon")
		TSubclassOf<class AProjectileBase> ProjectileClass;

protected:
	virtual void ClearAllTimerHandle() override;

	UPROPERTY()
		FTimerHandle AutoReloadTimerHandle;
};
