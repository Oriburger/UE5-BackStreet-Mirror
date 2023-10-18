// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "../public/RangedWeaponBase.h"
#include "ThrowWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API AThrowWeaponBase : public ARangedWeaponBase
{
	GENERATED_BODY()

public:
	AThrowWeaponBase();
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

//------ ��ô ���� ----------------------------------
public:
	//ThrowDirection���� �߻�
	UFUNCTION()
		void Throw();
	
	//ThrowDirection, ThrowEndLocation �� Ȱ���Ͽ� �߻� ���� �������� ����
	UFUNCTION()
		FPredictProjectilePathResult GetProjectilePathPredictResult();

	//SetThrowEndLocation ȣ�� ���Ŀ� ȣ�� �� ��
	//ThrowEndLocation ������ �߻簢�� ���Ͽ� ThrowDirection ������Ƽ�� ������Ʈ
	UFUNCTION()
		void CalculateThrowDirection(FVector NewDestination);

	UFUNCTION()
		FVector GetThrowDirection() { return ThrowDirection; }

private:
	//�Ź� ������Ʈ �� ���� / �߻�ü�� ������ ��ġ
		FVector ThrowDestination;

	//�Ź� ������Ʈ �� ���� / �߻�ü�� �߻�� ����
		FVector ThrowDirection;

	//���������� ���� �߻�ü�� ���� ����
		FProjectileStatStruct ProjectileStat;
};
