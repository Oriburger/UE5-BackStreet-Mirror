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

//------ Ranged �������̴��� ----------------------------
public:
	virtual bool TryFireProjectile();

	//������ �õ�. ���� ���¿� ���� ���� ���θ� ��ȯ
	virtual void Reload();

	//źȯ�� ������ ���� (ExtraAmmoCount����)
	virtual void AddAmmo(int32 Count);

//------ �⺻ ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		virtual void UpdateWeaponStat(FWeaponStatStruct NewStat) override;

	UFUNCTION(BlueprintCallable)
		class AProjectileBase* CreateProjectile();

//------- Getter / Setter ---------------------------
public:
	UFUNCTION()
		void SetInfiniteAmmoMode(bool NewMode) { WeaponStat.RangedWeaponStat.bIsInfiniteAmmo = NewMode; }

	//���� źȯ�� ������ ��ȯ - Stat.ExtraAmmoCount
	UFUNCTION(BlueprintCallable)
		int32 GetLeftAmmoCount() { return WeaponState.RangedWeaponState.ExtraAmmoCount + WeaponState.RangedWeaponState.CurrentAmmoCount; };

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetCanReload();

//------ Asset----------------------------------
protected:
	UFUNCTION()
		virtual void InitWeaponAsset() override;

	//�߻� ������ ��µ� �̹��� (�ӽ�, ���� ������ ���̺�� ���� ����)
	UPROPERTY(VisibleInstanceOnly)
		class UNiagaraSystem* ShootNiagaraEmitter;

	//����ü�� �߻�Ǵ� ����Ʈ�� ����Ѵ�
	UFUNCTION()
		void SpawnShootNiagaraEffect();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* ShootSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* ShootFailSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* ReloadSound;

protected:
	//�߻�ü�� ���� ���̺�
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Weapon|Projectile")
		UDataTable* ProjectileAssetInfoTable;

	//�߻�ü�� ���� ���̺�
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Weapon|Projectile")
		UDataTable* ProjectileStatInfoTable;

	//�߻�ü�� ���� ������ ���� ĳ�� ����
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileAssetInfoStruct ProjectileAssetInfo;

	//�߻�ü�� ������ ���� ĳ�� ����
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileStatStruct ProjectileStatInfo;

	UFUNCTION()
		FProjectileStatStruct GetProjectileStatInfo(int32 TargetProjectileID);

	UFUNCTION()
		FProjectileAssetInfoStruct GetProjectileAssetInfo(int32 TargetProjectileID);
	
//--------Ÿ�̸� ����--------------------
protected:
	virtual void ClearAllTimerHandle() override;

	UPROPERTY()
		FTimerHandle AutoReloadTimerHandle;
};
