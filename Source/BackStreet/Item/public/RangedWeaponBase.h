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
	ARangedWeaponBase();
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

//------ Asset----------------------------------
protected:
	UFUNCTION()
		virtual void InitWeaponAsset() override;

	//발사 순간에 출력될 이미터 (임시, 추후 데이터 테이블로 관리 예정)
	UPROPERTY(VisibleInstanceOnly)
		class UNiagaraSystem* ShootNiagaraEmitter;

	//투사체가 발사되는 이펙트를 출력한다
	UFUNCTION()
		void SpawnShootNiagaraEffect();

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

	//남은 탄환의 개수를 반환 - Stat.ExtraAmmoCount
	UFUNCTION(BlueprintCallable)
		int32 GetLeftAmmoCount() { return WeaponState.RangedWeaponState.ExtraAmmoCount + WeaponState.RangedWeaponState.CurrentAmmoCount; };

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetCanReload();

	//탄환의 개수를 더함 (ExtraAmmoCount까지)
	UFUNCTION(BlueprintCallable)
		void AddAmmo(int32 Count);

	//탄창의 개수를 더함 (+= MaxAmmoPerMagazine * Count)
	UFUNCTION()
		void AddMagazine(int32 Count);

	UFUNCTION()
		void SetInfiniteAmmoMode(bool NewMode) { WeaponStat.RangedWeaponStat.bIsInfiniteAmmo = NewMode; }

//--------Projectile Asset 관련 -----------------
protected:
	//발사체의 에셋 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Weapon|Projectile")
		UDataTable* ProjectileAssetInfoTable;

	//발사체의 에셋 정보를 담을 캐시 변수
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileAssetInfoStruct ProjectileAssetInfo;

private:
	UFUNCTION()
		FProjectileAssetInfoStruct GetProjectileAssetInfo(int32 TargetProjectileID);

//--------타이머 관련--------------------
protected:
	virtual void ClearAllTimerHandle() override;

	UPROPERTY()
		FTimerHandle AutoReloadTimerHandle;
};
