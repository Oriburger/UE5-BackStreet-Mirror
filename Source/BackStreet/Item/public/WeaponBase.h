// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
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

	//Weapon이 파괴되었을때 호출할 이벤트
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

//------- 기본 프로퍼티, Action -------------------
public:
	//공격 처리
	virtual void Attack();

	//공격 마무리 처리
	virtual void StopAttack();

	//필수 지정 프로퍼티
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Basic")
		int32 WeaponID;

	//무기를 초기화 한다. (스탯은 반드시 따로 초기화 해주어야한다.)
	UFUNCTION()
		void InitWeapon(int32 NewWeaponID);

//--------- 데이터 테이블, 에셋 관련 ----------------------
protected:
	//무기 에셋 정보 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Data")
		UDataTable* WeaponAssetInfoTable;
	
	//무기 스탯 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Data")
		UDataTable* WeaponStatInfoTable;

	UFUNCTION()
		void InitWeaponAsset();

	//현재 무기의 에셋 정보
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Asset")
		FWeaponAssetInfoStruct WeaponAssetInfo;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponAssetInfoStruct GetWeaponAssetInfoWithID(int32 TargetWeaponID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStatStruct GetWeaponStatInfoWithID(int32 TargetWeaponID);

//------ 스탯/상태 관련 ---------------------------------
public:
	//Weapon Stat 초기화
	virtual void UpdateWeaponStat(FWeaponStatStruct NewStat);

	UFUNCTION()
		void RevertWeaponInfo(FWeaponStatStruct OldWeaponStat, FWeaponStateStruct OldWeaponState);

	UFUNCTION()
		void UpdateDurabilityState();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsWeaponRangeType() { return WeaponStat.WeaponType == EWeaponType::E_Shoot || WeaponStat.WeaponType == EWeaponType::E_Throw; }

	//공격 범위를 반환
	virtual float GetAttackRange();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStatStruct GetWeaponStat() { return WeaponStat; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FWeaponStateStruct GetWeaponState() { return WeaponState; }

	UFUNCTION()
		void SetWeaponStat(FWeaponStatStruct NewStat) { WeaponStat = NewStat; }

	UFUNCTION()
		void SetWeaponState(FWeaponStateStruct NewState) { WeaponState = NewState; }

protected:
	//Weapon의 종합 Stat
	UPROPERTY(EditInstanceOnly, Category = "Gameplay|Stat")
		FWeaponStatStruct WeaponStat;

	//Weapon 상태 정보
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay|Stat")
		FWeaponStateStruct WeaponState;

//-------- 콤보 관련 ------------------------------------
public:
	//콤보 증가
	UFUNCTION(BlueprintCallable)
		void UpdateComboState();

	//Melee Combo 초기화하는 타이머를 등록
	UFUNCTION(BlueprintCallable)
		virtual void SetResetComboTimer();

	//현재 Combo 수를 반환 
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCurrentComboCnt() { return WeaponState.ComboCount; }

//-------- 그 외 (Ref, VFX 등)-------------------------------
public:
	UFUNCTION(BlueprintCallable)
		void SetOwnerCharacter(class ACharacterBase* NewOwnerCharacterRef);

protected:
	UFUNCTION()
		void PlayEffectSound(class USoundCue* EffectSound);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraComponent* MeleeTrailParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		FColor MeleeTrailParticleColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UParticleSystem* HitEffectParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UParticleSystem* DestroyEffectParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* HitImpactSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* AttackSound;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class USoundCue* AttackFailSound;

protected:
	//모든 타이머를 해제
	UFUNCTION(BlueprintCallable)
		virtual void ClearAllTimerHandle();

	UPROPERTY()
		FTimerHandle ComboTimerHandle;

protected:
	//게임모드 약 참조
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//소유자 캐릭터 약 참조
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};