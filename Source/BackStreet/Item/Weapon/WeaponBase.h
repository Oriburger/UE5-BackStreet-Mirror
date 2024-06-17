// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"
#define MAX_AMMO_LIMIT_CNT 2000

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateWeaponDestroy);

UCLASS()
class BACKSTREET_API AWeaponBase : public AActor
{
	GENERATED_BODY()

	friend class ACharacterBase; 

//------ Global, Component -------------------
public:
	// Sets default values for this actor's properties
	AWeaponBase();

	//Weapon�� �ı��Ǿ����� ȣ���� �̺�Ʈ
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateWeaponDestroy OnWeaponBeginDestroy;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleDefaultsOnly)
		USceneComponent* DefaultSceneRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UStaticMeshComponent* WeaponMesh;

//------- �⺻ ������Ƽ, Action -------------------
public:
	//���� ó��
	virtual void Attack();

	//���� ������ ó��
	virtual void StopAttack();

	//�ʼ� ���� ������Ƽ
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		int32 WeaponID;

	//���⸦ �ʱ�ȭ �Ѵ�. (������ �ݵ�� ���� �ʱ�ȭ ���־���Ѵ�.)
	UFUNCTION()
		void InitWeapon(int32 NewWeaponID);

//--------- ������ ���̺�, ���� ���� ----------------------
protected:
	//���� ���� ���� ���̺�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* WeaponAssetInfoTable;
	
	//���� ���� ���̺�
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* WeaponStatInfoTable;

	virtual void InitWeaponAsset();

	//���� ������ ���� ����
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,  Category = "Gameplay|Asset")
		FWeaponAssetInfoStruct WeaponAssetInfo;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponAssetInfoStruct GetWeaponAssetInfoWithID(int32 TargetWeaponID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStatStruct GetWeaponStatInfoWithID(int32 TargetWeaponID);

//------ ����/���� ���� ---------------------------------
public:
	//Weapon Stat �ʱ�ȭ
	virtual void UpdateWeaponStat(FWeaponStatStruct NewStat);

	UFUNCTION()
		void RevertWeaponInfo(FWeaponStatStruct OldWeaponStat, FWeaponStateStruct OldWeaponState);

	UFUNCTION()
		void UpdateDurabilityState();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsWeaponRangeType() { return WeaponStat.WeaponType == EWeaponType::E_Shoot || WeaponStat.WeaponType == EWeaponType::E_Throw; }

	//���� ������ ��ȯ
	virtual float GetAttackRange();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStatStruct GetWeaponStat() { return WeaponStat; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStateStruct GetWeaponState() { return WeaponState; }

	UFUNCTION(BlueprintCallable)
		void SetWeaponStat(FWeaponStatStruct NewStat) { WeaponStat = NewStat; }

	UFUNCTION(BlueprintCallable)
		void SetWeaponState(FWeaponStateStruct NewState) { WeaponState = NewState; }

	UFUNCTION()
		EWeaponType GetWeaponType();

	//Calculate total damage to target character
	UFUNCTION()
		float CalculateTotalDamage(FCharacterStateStruct TargetState);

	//Weapon�� ���� Stat
	UPROPERTY(EditInstanceOnly, Category = "Gameplay|Stat")
		FWeaponStatStruct WeaponStat;

	//Weapon ���� ����
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Stat")
		FWeaponStateStruct WeaponState;

//-------- �޺� ���� ------------------------------------
public:
	//�޺� ����
	UFUNCTION(BlueprintCallable)
		void UpdateComboState();

	//Melee Combo �ʱ�ȭ�ϴ� Ÿ�̸Ӹ� ���
	UFUNCTION(BlueprintCallable)
		virtual void SetResetComboTimer();

	//���� Combo ���� ��ȯ 
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentComboCnt() { return WeaponState.ComboCount; }

private:
	UFUNCTION()
		void ResetComboCnt() { WeaponState.ComboCount = 0; }

//-------- �� �� (Ref, VFX ��)-------------------------------
public:
	UFUNCTION(BlueprintCallable)
		void SetOwnerCharacter(class ACharacterBase* NewOwnerCharacterRef);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UParticleSystem* DestroyEffectParticle;

protected:
	//��� Ÿ�̸Ӹ� ����
	UFUNCTION(BlueprintCallable)
		virtual void ClearAllTimerHandle();

	UPROPERTY()
		FTimerHandle ComboTimerHandle;

protected:
	//���Ӹ�� �� ����
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//������ ĳ���� �� ����
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;
};