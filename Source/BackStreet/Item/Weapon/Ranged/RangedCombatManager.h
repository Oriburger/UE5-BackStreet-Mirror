// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "../CombatManager.h"
#include "RangedCombatManager.generated.h"


UCLASS()
class BACKSTREET_API URangedCombatManager : public UCombatManager
{
	GENERATED_BODY()
public:
	URangedCombatManager();
	


public:
	//���� ó��		
	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	//���� ������ ó��
	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

//------ Ranged �������̴��� ----------------------------
public:
	UFUNCTION(BlueprintCallable)
		bool TryFireProjectile(FRotator FireRotationOverride = FRotator::ZeroRotator);
	
	//Add ammo count
	UFUNCTION(BlueprintCallable)
		void AddAmmo(int32 Count);

//------ Basic ---------------------------------
protected:
	UFUNCTION(BlueprintCallable)
		class AProjectileBase* CreateProjectile(FRotator FireRotationOverride = FRotator::ZeroRotator);

//------- Getter / Setter ---------------------------
public:
	UFUNCTION()
		void SetInfiniteAmmoMode(bool NewMode);

	UFUNCTION(BlueprintCallable)
		int32 GetLeftAmmoCount();

//------ Asset----------------------------------
protected:
	//�߻� ������ ��µ� �̹��� (�ӽ�, ���� ������ ���̺�� ���� ����)
	UPROPERTY(VisibleInstanceOnly)
		class UNiagaraSystem* ShootNiagaraEmitter;

	//����ü�� �߻�Ǵ� ����Ʈ�� ����Ѵ�
	UFUNCTION()
		void SpawnShootNiagaraEffect();

//--------Ÿ�̸� ����--------------------
private:
	//Ÿ�̸� �̺�Ʈ ������ ���ؼ��� ���
	//��������Ʈ ���ε� ���� �Ķ���͸� ���� �����ϸ�
	//Ÿ�̸� �̺�Ʈ ���� ���߿� �Ķ���Ͱ� �޸𸮿��� �Ҹ�Ǳ⿡ �� ���� �߻�
	FRotator FireRotationForTimer; 
};

