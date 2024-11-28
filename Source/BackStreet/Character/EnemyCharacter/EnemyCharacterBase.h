// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../CharacterBase.h"
#include "EnemyCharacterBase.generated.h"

DECLARE_DELEGATE_OneParam(FDelegateEnemyDeath, class AEnemyCharacterBase*);
DECLARE_DELEGATE_OneParam(FDelegateEnemyDamage, class AActor*);

UCLASS()
class BACKSTREET_API AEnemyCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

// ------ Global, Component ------------
public:
	AEnemyCharacterBase();
	
	//�� Death �̺�Ʈ
	FDelegateEnemyDeath EnemyDeathDelegate;

	//�� Hit �̺�Ʈ
	FDelegateEnemyDamage EnemyDamageDelegate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

// ------- ������Ʈ ----------
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|UI")
		class UWidgetComponent* FloatingHpBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|UI")
		class UWidgetComponent* TargetingSupportWidget;

// ----- Asset & Stat ---------------------
	UFUNCTION(BlueprintCallable)
		virtual void InitAsset(int32 NewCharacterID) override;	

	//Stat data of enemy character including stat, default weapon, drop info
	//EnemyStatStruct.EnemyStat member initializes the parent's member CharacterStat
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Gameplay")
		FEnemyStatStruct EnemyStat;

// ----- Action ---------------
public:
	UFUNCTION()
		virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
			, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
		virtual void TryAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual bool TrySkill(int32 SkillID) override;

	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION()
		virtual void Die() override;

	//�÷��̾ �߰��� ���� ���� Action(Anim), ��� �ð��� ��ȯ
	UFUNCTION(BlueprintCallable)
		float PlayPreChaseAnimation();

protected:
	virtual void KnockDown() override;

	UFUNCTION()
		virtual void StandUp() override;

// -------- �����̺��Ʈ�� �� AIController ���� ---------
public:
	//Calculate probability depend on HitCount
	UFUNCTION()
	bool CalculateIgnoreHitProbability(int32 HitCounter);


// ----- ĳ���� ���� �� ���� ���� ---------
public:
	//Initialize EnemyCharacter except asset info.
	//+) It is better to use virtual function 'initCharacter' to all character classes (enemy, main, parent)
	UFUNCTION(BlueprintCallable)
		void InitEnemyCharacter(int32 NewCharacterID);

	//���� ���� ���̺�
	UPROPERTY(EditDefaultsOnly, Category = "Data|Table")
		UDataTable* EnemyStatTable;

private:
	//Set default weapon actor using weapon inventory
	UFUNCTION()
		void SetDefaultWeapon();

	UFUNCTION()
		void SpawnDeathItems();
	
	UFUNCTION()
		void ResetActionStateForTimer();

private:
	UPROPERTY()
		float DefaultKnockBackStrength = 2000.0f;

// ---- VFX ---------------------
public:
	UFUNCTION(BlueprintCallable)
		void SetFacialMaterialEffect(bool NewState);

// ---- �� �� (����, ���� ��) ----
protected:
	UFUNCTION(BlueprintImplementableEvent)
		void InitFloatingHpWidget();

	UFUNCTION(BlueprintNativeEvent)
		void InitTargetingSupportingWidget();

	UFUNCTION(BlueprintImplementableEvent)
		void OnTargetUpdated(APawn* Target);

	UFUNCTION(BlueprintImplementableEvent)
		void OnTargetingActivated(bool bIsActivated, APawn* Target);

private:
	void SetInstantHpWidgetVisibility();

//----- Timer ----------------
public:
	void ClearAllTimerHandle();

private:
	//���� Turn�� ������ �ʰ� TimeOut ó�� �����ִ� Ÿ�̸� �ڵ�
	FTimerHandle TurnTimeOutTimerHandle;

	FTimerHandle HitTimeOutTimerHandle;

	FTimerHandle DamageAIDelayTimer;

	FTimerHandle HpWidgetAutoDisappearTimer;
};
