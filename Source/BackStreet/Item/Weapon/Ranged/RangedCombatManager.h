// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "../CombatManager.h"
#include "RangedCombatManager.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API URangedCombatManager : public UCombatManager
{
	GENERATED_BODY()
public:
	URangedCombatManager();

	//���� ó��		
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//���� ������ ó��
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

//------ Ranged �������̴��� ----------------------------
public:
	UFUNCTION(BlueprintCallable)
		bool TryFireProjectile();

	//������ �õ�. ���� ���¿� ���� ���� ���θ� ��ȯ
	UFUNCTION(BlueprintCallable)
		void Reload();

	//Add ammo count (til ExtraAmmoCount)
	UFUNCTION(BlueprintCallable)
		void AddAmmo(int32 Count);

//------ Basic ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		class AProjectileBase* CreateProjectile();

//------- Getter / Setter ---------------------------
public:
	UFUNCTION()
		void SetInfiniteAmmoMode(bool NewMode);

	//get Stat.ExtraAmmoCount
	UFUNCTION(BlueprintCallable)
		int32 GetLeftAmmoCount();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetCanReload();

	//------ Asset----------------------------------
protected:
	//�߻� ������ ��µ� �̹��� (�ӽ�, ���� ������ ���̺�� ���� ����)
	UPROPERTY(VisibleInstanceOnly)
		class UNiagaraSystem* ShootNiagaraEmitter;

	//����ü�� �߻�Ǵ� ����Ʈ�� ����Ѵ�
	UFUNCTION()
		void SpawnShootNiagaraEffect();

public:
	//�߻�ü�� ���� ���̺�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* ProjectileAssetInfoTable;

	//�߻�ü�� ���� ���̺�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* ProjectileStatInfoTable;

protected:
	//�߻�ü�� ���� ������ ���� ĳ�� ����
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileAssetInfoStruct ProjectileAssetInfo;

	//�߻�ü�� ������ ���� ĳ�� ����
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Weapon|Projectile")
		FProjectileStatStruct ProjectileStatInfo;

	UFUNCTION()
		FProjectileStatStruct GetProjectileStatInfo(int32 TargetProjectileID);

	UFUNCTION()
		FProjectileAssetInfoStruct GetProjectileAssetInfo(int32 TargetProjectileID);

	//--------Ÿ�̸� ����--------------------
protected:
	UPROPERTY()
		FTimerHandle AutoReloadTimerHandle;
};

