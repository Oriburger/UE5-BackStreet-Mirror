// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "CharacterBase.h"
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

// ----- 기본 프로퍼티 -----------
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		int32 EnemyID;

	//적이 최초로 소유하는 무기의 ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
		int32 DefaultWeaponID;

	//최대 스폰할 아이템의 개수. 미션 아이템은 무시.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem", meta = (UIMin = 0, UIMax = 2))
		int32 MaxSpawnItemCount;

	//적이 죽고 스폰할 아이템의 Type 리스트 (각 아이템은 Idx로 구별)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<EItemCategoryInfo> SpawnItemTypeList;

	//적이 죽구 스폰할 아이템 ID 리스트 (각 아이템은 Idx로 구별)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<uint8> SpawnItemIDList;

	//적이 죽고 스폰할 아이템의 스폰 확률 리스트  (0.0f ~ 1.0f),  (각 아이템은 Idx로 구별)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|DropItem")
		TArray<float> ItemSpawnProbabilityList;

// ----- Action ---------------
public:
	UFUNCTION()
		virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
			, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
		virtual void TryAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION()
		virtual void Die() override;

	UFUNCTION(BlueprintCallable)
		void Turn(float Angle);

	//플레이어를 발견한 직후 취할 Action(Anim), 재생 시간을 반환
	UFUNCTION(BlueprintCallable)
		float PlayPreChaseAnimation();

// ----- 캐릭터 스탯 및 상태 관련 ---------
public:
	UFUNCTION(BlueprintCallable)
		void InitEnemyStat();

protected:
	UFUNCTION()
		void SetDefaultWeapon();

	UFUNCTION()
		void SetDefaultStat();

	UFUNCTION()
		void SpawnDeathItems();

// ---- VFX ---------------------
public:
	UFUNCTION(BlueprintCallable)
		void SetFacialMaterialEffect(bool NewState);

	//테스트 코드 -> 상수화 예정
	UPROPERTY(EditDefaultsOnly)
		float knockBackStrength = 100000.0f;

// ---- 그 외 (위젯, 사운드 등) ----
public:
	UFUNCTION(BlueprintImplementableEvent)
		void InitFloatingHpWidget();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		USoundCue* HitImpactSound;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Animation")
		class UAnimMontage* PreChaseAnimMontage;

public:
	void ClearAllTimerHandle();

private:
	//무한 Turn에 빠지지 않게 TimeOut 처리 시켜주는 타이머 핸들
	UPROPERTY()
		FTimerHandle TurnTimeOutTimerHandle;
};
