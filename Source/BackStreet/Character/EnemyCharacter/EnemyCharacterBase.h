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
	
	//적 Death 이벤트
	FDelegateEnemyDeath EnemyDeathDelegate;

	//적 Hit 이벤트
	FDelegateEnemyDamage EnemyDamageDelegate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|UI")
		class UWidgetComponent* FloatingHpBar;

protected:
	//Stat data of enemy character including stat, default weapon, drop info
	//EnemyStatStruct.EnemyStat member initializes the parent's member CharacterStat
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay")
		FEnemyStatStruct EnemyStat;

// ----- Action ---------------
public:
	UFUNCTION()
		virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
			, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
		virtual void TryAttack() override;

	//temporary code
	UFUNCTION(BlueprintImplementableEvent)
		void BossNearAttack();

	UFUNCTION(BlueprintCallable)
		virtual void TrySkill(ESkillType SkillType, int32 SkillID) override;

	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION()
		virtual void Die() override;

	//플레이어를 발견한 직후 취할 Action(Anim), 재생 시간을 반환
	UFUNCTION(BlueprintCallable)
		float PlayPreChaseAnimation();

protected:
	virtual void KnockDown() override;

//-------- Inventory -------------
public:
	UFUNCTION(BlueprintCallable)
		virtual bool PickWeapon(int32 NewWeaponID) override;

	UFUNCTION(BlueprintCallable)
	virtual void SwitchToNextWeapon() override;

// ----- 캐릭터 스탯 및 상태 관련 ---------
public:
	//Initialize EnemyCharacter except asset info.
	//+) It is better to use virtual function 'initCharacter' to all character classes (enemy, main, parent)
	UFUNCTION(BlueprintCallable)
		void InitEnemyCharacter(int32 NewCharacterID);

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

	//적의 스탯 테이블
	UPROPERTY()
		UDataTable* EnemyStatTable;

// ---- VFX ---------------------
public:
	UFUNCTION(BlueprintCallable)
		void SetFacialMaterialEffect(bool NewState);

// ---- 그 외 (위젯, 사운드 등) ----
public:
	UFUNCTION(BlueprintImplementableEvent)
		void InitFloatingHpWidget();

public:
	void ClearAllTimerHandle();

private:
	//무한 Turn에 빠지지 않게 TimeOut 처리 시켜주는 타이머 핸들
	UPROPERTY()
		FTimerHandle TurnTimeOutTimerHandle;

	UPROPERTY()
		FTimerHandle HitTimeOutTimerHandle;

	UPROPERTY()
		FTimerHandle DamageAIDelayTimer;
};
