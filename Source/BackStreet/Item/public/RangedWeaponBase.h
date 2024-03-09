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
	
	virtual void Attack();

	virtual void StopAttack();

	virtual float GetAttackRange();

//------ Ranged 오버라이더블 ----------------------------
public:
	virtual bool TryFireProjectile();

	//장전을 시도. 현재 상태에 따른 성공 여부를 반환
	virtual void Reload();

	//탄환의 개수를 더함 (ExtraAmmoCount까지)
	virtual void AddAmmo(int32 Count);

//------ 기본 ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		virtual void UpdateWeaponStat(FWeaponStatStruct NewStat) override;

	UFUNCTION(BlueprintCallable)
		class AProjectileBase* CreateProjectile();

//------- Getter / Setter ---------------------------
public:
	UFUNCTION()
		void SetInfiniteAmmoMode(bool NewMode) { WeaponStat.RangedWeaponStat.bIsInfiniteAmmo = NewMode; }

	//남은 탄환의 개수를 반환 - Stat.ExtraAmmoCount
	UFUNCTION(BlueprintCallable)
		int32 GetLeftAmmoCount() { return WeaponState.RangedWeaponState.ExtraAmmoCount + WeaponState.RangedWeaponState.CurrentAmmoCount; };

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetCanReload();

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

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* ShootSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* ShootFailSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* ReloadSound;

protected:
	//발사체의 에셋 테이블
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Weapon|Projectile")
		UDataTable* ProjectileAssetInfoTable;

	//발사체의 스탯 테이블
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Weapon|Projectile")
		UDataTable* ProjectileStatInfoTable;

	//발사체의 에셋 정보를 담을 캐시 변수
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileAssetInfoStruct ProjectileAssetInfo;

	//발사체의 스탯을 담을 캐시 변수
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileStatStruct ProjectileStatInfo;

	UFUNCTION()
		FProjectileStatStruct GetProjectileStatInfo(int32 TargetProjectileID);

	UFUNCTION()
		FProjectileAssetInfoStruct GetProjectileAssetInfo(int32 TargetProjectileID);
	
//--------타이머 관련--------------------
protected:
	virtual void ClearAllTimerHandle() override;

	UPROPERTY()
		FTimerHandle AutoReloadTimerHandle;
};
