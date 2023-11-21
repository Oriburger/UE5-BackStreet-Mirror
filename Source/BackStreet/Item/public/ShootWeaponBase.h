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

	//���� ó��
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//���� ������ ó��
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual float GetAttackRange() override;

	UFUNCTION(BlueprintCallable)
		virtual bool TryFireProjectile() override;

	//������ �õ�. ���� ���¿� ���� ���� ���θ� ��ȯ
	UFUNCTION(BlueprintCallable)
		virtual bool TryReload() override;

	//źȯ�� ������ ���� (ExtraAmmoCount����)
	UFUNCTION(BlueprintCallable)
		virtual void AddAmmo(int32 Count) override;

//------ Shoot ���� ----------------------------------
public:
	//źâ�� ������ ���� (+= MaxAmmoPerMagazine * Count)
	UFUNCTION(BlueprintCallable)
		void AddMagazine(int32 Count);
};
