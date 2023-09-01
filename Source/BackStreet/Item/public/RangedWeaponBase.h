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
	//���� ó��
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//���� ������ ó��
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual float GetAttackRange() override;

//------ ����/���� ���� ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		virtual void UpdateWeaponStat(FWeaponStatStruct NewStat) override;

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

//------ Projectile ����----------------------------
public:
	//�߻�ü�� ����
	UFUNCTION()
		class AProjectileBase* CreateProjectile();

	//�߻�ü �ʱ�ȭ �� �߻縦 �õ� 
	UFUNCTION()
		bool TryFireProjectile();

	//������ �õ�. ���� ���¿� ���� ���� ���θ� ��ȯ
	UFUNCTION(BlueprintCallable)
		bool TryReload();

	//���� źȯ�� ������ ��ȯ - Stat.ExtraAmmoCount
	UFUNCTION(BlueprintCallable)
		int32 GetLeftAmmoCount() { return WeaponState.RangedWeaponState.ExtraAmmoCount + WeaponState.RangedWeaponState.CurrentAmmoCount; };

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetCanReload();

	//źȯ�� ������ ���� (ExtraAmmoCount����)
	UFUNCTION(BlueprintCallable)
		void AddAmmo(int32 Count);

	//źâ�� ������ ���� (+= MaxAmmoPerMagazine * Count)
	UFUNCTION()
		void AddMagazine(int32 Count);

	UFUNCTION()
		void SetInfiniteAmmoMode(bool NewMode) { WeaponStat.RangedWeaponStat.bIsInfiniteAmmo = NewMode; }

//--------Projectile Asset ���� -----------------
protected:
	//�߻�ü�� ���� ���̺�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Weapon|Projectile")
		UDataTable* ProjectileAssetInfoTable;

	//�߻�ü�� ���� ������ ���� ĳ�� ����
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileAssetInfoStruct ProjectileAssetInfo;

private:
	UFUNCTION()
		FProjectileAssetInfoStruct GetProjectileAssetInfo(int32 TargetProjectileID);

//--------Ÿ�̸� ����--------------------
protected:
	virtual void ClearAllTimerHandle() override;

	UPROPERTY()
		FTimerHandle AutoReloadTimerHandle;
};
