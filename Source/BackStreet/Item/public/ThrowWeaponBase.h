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

	UPROPERTY(EditInstanceOnly)
		class USplineComponent* ProjectilePathSpline;

public:
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
		virtual void Reload();

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector GetThrowDirection() { return ThrowDirection; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector GetThrowDestination() { return ThrowDestination; }

private:
	//�Ź� ������Ʈ �� ���� / �߻�ü�� ������ ��ġ
		FVector ThrowDestination;

	//�Ź� ������Ʈ �� ���� / �߻�ü�� �߻�� ����
		FVector ThrowDirection;

	//�Ź� ������Ʈ �� ���� / �߻�ü�� ���ư� �ӷ�
		float ThrowSpeed;

	//���������� ���� �߻�ü�� ���� ����
		FProjectileStatStruct ProjectileStat;
		
//------ Projectile Path Spline -------------------
public:
	UFUNCTION()
		void UpdateProjectilePathSpline(TArray<FVector>& Locations);

	UFUNCTION()
		void ClearProjectilePathSpline();

private:
	UPROPERTY()
		TArray<class USplineMeshComponent*> SplineMeshComponentList;

	UPROPERTY()
		class UStaticMesh* SplineMesh;

	UPROPERTY()
		class UMaterialInterface* SplineMeshMaterial;

	UPROPERTY()
		class UMaterialInterface* PathTargetDecalMaterial; 

//------- Ÿ�̸� ���� -------------------------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetThrowDelayRemainingTime();

	UFUNCTION(BlueprintCallable, BlueprintPure)		
		bool GetCanThrow() { return GetThrowDelayRemainingTime() == 0; }

private:
	//��ô ������ Ÿ�̸� �ڵ�
		FTimerHandle ThrowDelayHandle;
};
