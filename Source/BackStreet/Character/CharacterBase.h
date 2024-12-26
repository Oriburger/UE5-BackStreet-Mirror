#pragma once

#include "../Global/BackStreet.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateHealthChange, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateBeginLocationInterp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateEndLocationInterp);
DECLARE_MULTICAST_DELEGATE(FDelegateOnActionTrigger);

UCLASS()
class BACKSTREET_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()


//========================================================================
//====== Delegate ========================================================
public:
	FDelegateOnActionTrigger OnMoveStarted;
	FDelegateOnActionTrigger OnJumpStarted;
	FDelegateOnActionTrigger OnJumpEnd;
	FDelegateOnActionTrigger OnRollStarted;
	FDelegateOnActionTrigger OnSprintStarted;
	FDelegateOnActionTrigger OnSprintEnd;
	FDelegateOnActionTrigger OnRollEnded;
	FDelegateOnActionTrigger OnAttackStarted;
	FDelegateOnActionTrigger OnDashAttackStarted;
	FDelegateOnActionTrigger OnJumpAttackStarted;
	FDelegateOnActionTrigger OnDamageReceived;
	FDelegateOnActionTrigger OnSkillStarted;
	FDelegateOnActionTrigger OnSkillEnded;
	FDelegateOnActionTrigger OnDeath;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateHealthChange OnHealthChanged;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateBeginLocationInterp OnBeginLocationInterp;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateEndLocationInterp OnEndLocationInterp;

public:
	TMap<FName, FDelegateOnActionTrigger*> ActionTriggerDelegateMap;

private:
	//생성자에서 호출
	void InitializeActionTriggerDelegateMap();


//========================================================================
//====== Global / Component ==============================================
public:
	// Sets default values for this character's properties
	ACharacterBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly)
		UChildActorComponent* InventoryComponent;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		USceneComponent* HitSceneComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UWeaponComponentBase* WeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UDebuffManagerComponent* DebuffManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UTargetingManagerComponent* TargetingManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UActionTrackingComponent* ActionTrackingComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class USkillManagerComponentBase* SkillManagerComponent;

//========================================================================
//========= Action =======================================================
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		int32 CharacterID;

	//Input에 Binding 되어 공격을 시도 (AnimMontage를 호출)
	virtual void TryAttack();

	//Try Upper Attack
	virtual	void TryUpperAttack();

	//Try Downer Attack
	virtual	void TryDownwardAttack();

	//try play dash atk montage
	UFUNCTION()
		void TryDashAttack();

	// called with notify, activate dash logic with interp
	UFUNCTION(BlueprintCallable)
		void DashAttack();

	///Input에 Binding 되어 스킬공격을 시도 (AnimMontage를 호출)
	virtual bool TrySkill(int32 SkillID);

	//AnimNotify에 Binding 되어 실제 공격을 수행
	virtual void Attack();

	virtual void StopAttack();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
		, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Die();

	virtual void TakeKnockBack(FVector KnockBackDirection, float Strength);

	//플레이어가 현재 해당 Action을 수행하고 있는지 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsActionActive(ECharacterActionType Type) { return CharacterGameplayInfo.CharacterActionState == Type; }

	//플레이어의 ActionState를 Idle로 전환한다.
	UFUNCTION(BlueprintCallable)
		void ResetActionState(bool bForceReset = false);

	//Set Player's Action State
	UFUNCTION(BlueprintCallable)
		void SetActionState(ECharacterActionType Type);

	//플레이어가 체력을 회복함 (일회성)
	UFUNCTION(BlueprintCallable)
		void TakeHeal(float HealAmount, bool bIsTimerEvent = false, uint8 BuffDebuffType = 0);

	//디버프 데미지를 입힘 (일회성)
	UFUNCTION(BlueprintCallable)
		float TakeDebuffDamage(ECharacterDebuffType DebuffType, float DamageAmount, AActor* Causer);

	UFUNCTION(BlueprintCallable)
		void ApplyKnockBack(AActor* Target, float Strength);

	//공격Action 사이의 Interval을 관리하는 타이머를 해제
	UFUNCTION()
		void ResetAtkIntervalTimer();

	//Update Character's world Location using interporlation
	UFUNCTION(BlueprintCallable)
		void SetLocationWithInterp(FVector NewValue, float InterpSpeed = 1.0f, const bool bAutoReset = false);

	UFUNCTION(BlueprintCallable)
		void ResetLocationInterpTimer();

	UFUNCTION()
		void PlayHitAnimMontage();

	UFUNCTION()
		void PlayKnockBackAnimMontage();

	UFUNCTION(BlueprintCallable)
		TArray<FName> GetCurrentMontageSlotName();
	
	//Set air atk location update event timer
	//This func is called by upper atk anim montage's notify
	UFUNCTION(BlueprintCallable)
		void SetAirAtkLocationUpdateTimer(); 

	//Disable air atk location update event timer
	//This func is called by downward atk anim montage's notify
	UFUNCTION(BlueprintCallable)
		void ResetAirAtkLocationUpdateTimer();

	////Launch event when upperattack
	//UFUNCTION(BlueprintCallable)
	//	void LaunchCharacterWithTarget();

	//UFUNCTION(BlueprintCallable)
	//	void ActivateAirMode(bool bDeactivateFlag = false);	

protected:
	//interp function
	//it must not be called alone.
	//if you set the value of bAutoReset false, you have to call this function to reset to original value
	UFUNCTION()
		void UpdateLocation(const FVector TargetValue, const float InterpSpeed = 1.0f, const bool bAutoReset = false);

	//Set distance from ground for air attack
	UFUNCTION()
		void SetAirAttackLocation();

	UFUNCTION()
		void OnPlayerLanded(const FHitResult& Hit);

	UFUNCTION()
		void ResetHitCounter();	

	virtual TArray<UAnimMontage*> GetTargetMeleeAnimMontageList();

	//Knock down with 
	virtual void KnockDown();

	//Stand UP
	virtual void StandUp();

//========================================================================
//====== Stat / State ====================================================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FCharacterGameplayInfo& GetCharacterGameplayInfoRef() { return CharacterGameplayInfo; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FCharacterGameplayInfo GetCharacterGameplayInfo() { return CharacterGameplayInfo; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetStatTotalValue(ECharacterStatType TargetStatType) { return CharacterGameplayInfo.GetTotalValue(TargetStatType); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetStatProbabilityValue(ECharacterStatType TargetStatType) { return CharacterGameplayInfo.GetProbabilityStatInfo(TargetStatType); }

	UFUNCTION(BlueprintCallable)
		void InitCharacterGameplayInfo(FCharacterGameplayInfo NewGameplayInfo);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetCurrentHP();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetMaxHP();

	//캐릭터의 디버프 정보를 업데이트
	UFUNCTION(BlueprintCallable)
		virtual	bool TryAddNewDebuff(FDebuffInfoStruct DebuffInfo, AActor* Causer);

	//디버프가 활성화 되어있는지 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetDebuffIsActive(ECharacterDebuffType DebuffType);

	UFUNCTION(BlueprintCallable)
		void UpdateWeaponStat(FWeaponStatStruct NewStat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetMaxComboCount() { return AssetHardPtrInfo.NormalComboAnimMontageList.Num(); }
		
//========================================================================
//====== Asset / Weapon ==================================================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool EquipWeapon(int32 NewWeaponID);
// ---- Asset -------------------
public:
	//Init asset using softref data table
	virtual void InitAsset(int32 NewCharacterID);

protected:
	UFUNCTION()
		void SetAsset();

	UFUNCTION()
		bool InitAnimAsset();

	UFUNCTION()
		void InitSoundAsset();

	UFUNCTION()
		void InitVFXAsset();

	UFUNCTION()
		void InitMaterialAsset();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FCharacterAssetSoftInfo GetAssetSoftInfoWithID(const int32 TargetCharacterID);

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		FCharacterAssetSoftInfo AssetSoftPtrInfo;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		FCharacterAssetHardInfo AssetHardPtrInfo;

	//적 데이터 테이블 (에셋 정보 포함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		UDataTable* AssetDataInfoTable;

public:
	UPROPERTY(BlueprintReadOnly)
		TArray<USoundCue*> FootStepSoundList;

//========================================================================
//====== VFX ============================================================
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraComponent* BuffNiagaraEmitter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraComponent* DirectionNiagaraEmitter;

public:
	UFUNCTION()
		void ActivateDebuffNiagara(uint8 DebuffType);

	UFUNCTION()
		void DeactivateBuffEffect();

//========================================================================
//====== PROPERTY ========================================================
protected:
	//캐릭터의 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		FCharacterDefaultStat DefaultStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
		FCharacterGameplayInfo CharacterGameplayInfo;

	//Gamemode 약 참조
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//Gamemode 약 참조
		TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;


//========================================================================
//====== Timer ===========================================================
protected:
	UFUNCTION()
		virtual void ClearAllTimerHandle();

	//공격 간 딜레이 핸들
	UPROPERTY()
		FTimerHandle AtkIntervalHandle;

	UPROPERTY()
		TArray<FTimerHandle> SkillAnimPlayTimerHandleList;

	UPROPERTY()
		FTimerHandle KnockDownDelayTimerHandle;	

	UPROPERTY()
		FTimerHandle KnockDownAnimMontageHandle;

	UPROPERTY()
		FTimerHandle AirAtkLocationUpdateHandle;

	UPROPERTY()
		FTimerHandle HitCounterResetTimerHandle;

	//Player Location Interpolate timer
	UPROPERTY()
		FTimerHandle LocationInterpHandle;

private:
	//Current number of multiple SkillAnimPlayTimers
	UPROPERTY()
		int SkillAnimPlayTimerCurr;

	//Threshold number of multiple SkillAnimPlayTimer
	UPROPERTY()
		int SkillAnimPlayTimerThreshold;

};