#pragma once

#include "../Global/BackStreet.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateCharacterDie);

UCLASS()
class BACKSTREET_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

	friend class AWeaponInventoryBase;

//----- Global / Component ----------
public:
	// Sets default values for this character's properties
	ACharacterBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Weapon이 파괴되었을때 호출할 이벤트
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateCharacterDie OnCharacterDied;

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

protected:
	UPROPERTY(VisibleAnywhere)
		class UDebuffManagerComponent* DebuffManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UTargetingManagerComponent* TargetingManagerComponent;

// ------- Character Action 기본 ------- 
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
	virtual void TrySkill(ESkillType SkillType, int32 SkillID);

	//AnimNotify에 Binding 되어 실제 공격을 수행
	virtual void Attack();

	virtual void StopAttack();

	virtual void TryReload();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
		, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Die();

	//플레이어가 현재 해당 Action을 수행하고 있는지 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsActionActive(ECharacterActionType Type) { return CharacterState.CharacterActionState == Type; }

	//플레이어의 ActionState를 Idle로 전환한다.
	UFUNCTION(BlueprintCallable)
		void ResetActionState(bool bForceReset = false);

	//Set Player's Action State
	UFUNCTION(BlueprintCallable)
		void SetActionState(ECharacterActionType Type);

	//플레이어가 체력을 회복함 (일회성)
	UFUNCTION()
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
	UFUNCTION()
		void SetLocationWithInterp(FVector NewValue, float InterpSpeed = 1.0f, const bool bAutoReset = false);

	//Set air atk location update event timer
	//This func is called by upper atk anim montage's notify
	UFUNCTION(BlueprintCallable)
		void SetAirAtkLocationUpdateTimer(); 

	//Disable air atk location update event timer
	//This func is called by downward atk anim montage's notify
	UFUNCTION(BlueprintCallable)
		void ResetAirAtkLocationUpdateTimer();

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

	//Knock down with 
	virtual void KnockDown();

// ------- Character Stat/State ------------------------------
public:
	//캐릭터의 상태 정보를 초기화
	UFUNCTION()
		void InitCharacterState();

	//캐릭터의 디버프 정보를 업데이트
	UFUNCTION(BlueprintCallable)
		virtual	bool TryAddNewDebuff(FDebuffInfoStruct DebuffInfo, AActor* Causer);

	//디버프가 활성화 되어있는지 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetDebuffIsActive(ECharacterDebuffType DebuffType);

	//Update Character's stat and state
	UFUNCTION(BlueprintCallable)
		void UpdateCharacterStatAndState(FCharacterStatStruct NewStat, FCharacterStateStruct NewState);

	//캐릭터의 스탯을 업데이트
	UFUNCTION(BlueprintCallable)
		void UpdateCharacterStat(FCharacterStatStruct NewStat);

	UFUNCTION(BlueprintCallable)
		void UpdateCharacterState(FCharacterStateStruct NewState);

	UFUNCTION(BlueprintCallable)
		void UpdateWeaponStat(FWeaponStatStruct NewStat);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FCharacterStatStruct GetCharacterStat() { return CharacterStat; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FCharacterStateStruct GetCharacterState() { return CharacterState; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetMaxComboCount() { return AssetHardPtrInfo.MeleeAttackAnimMontageList.Num(); }

private:
	//Calculate Total Stat Value
	UFUNCTION()
		float GetTotalStatValue(float& DefaultValue, FStatInfoStruct& AbilityInfo, FStatInfoStruct& SkillInfo, FStatInfoStruct& DebuffInfo);

// ------ 무기 관련 -------------------------------------------
public:
	UFUNCTION()
		bool EquipWeapon(class AWeaponBase* TargetWeapon);

	//무기를 집는다. 인벤토리가 꽉 찼다면 false를 반환
	virtual bool PickWeapon(int32 NewWeaponID);

	//다음 무기로 전환한다. 전환에 실패하면 false를 반환
	virtual void SwitchToNextWeapon();

	//무기를 Drop한다. (월드에서 아예 사라진다.)
	virtual void DropWeapon();

	UFUNCTION()
		bool TrySwitchToSubWeapon(int32 SubWeaponIdx);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AWeaponInventoryBase* GetWeaponInventoryRef();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AWeaponInventoryBase* GetSubWeaponInventoryRef();

	//무기 Ref를 반환
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AWeaponBase* GetCurrentWeaponRef();

protected:
	UPROPERTY()
		TSubclassOf<class AWeaponInventoryBase> WeaponInventoryClass;

	UFUNCTION()
		void SwitchWeaponActor(EWeaponType TargetWeaponType);

private:
	UPROPERTY()
		class AWeaponInventoryBase* WeaponInventoryRef;

	UPROPERTY()
		class AWeaponInventoryBase* SubWeaponInventoryRef;

	//0번째 : 들고 있는 무기 / 1번째, 숨겨져 있는 다른 타임의무기
	UPROPERTY()
		TArray<class AWeaponBase*> WeaponActorList;

protected:
	UPROPERTY()
		TWeakObjectPtr<class AWeaponBase> CurrentWeaponRef;

	//0번째 : 근접 무기 / 1번째 : 원거리 무기
	//하나의 WeaponBase로 통일을 한다면 이렇게 하지 않아도 될텐데..

private:
	UPROPERTY()
		TArray<TSubclassOf<class AWeaponBase>> WeaponClassList; 

	//초기 무기 액터들을 스폰하고 초기화 한다.
	void InitWeaponActors();

	//무기 액터를 스폰
	AWeaponBase* SpawnWeaponActor(EWeaponType TargetWeaponType);

// ---- Asset -------------------
public:
	// 외부에서 Init하기위해 Call
	UFUNCTION(BlueprintCallable)
		void InitAsset(int32 NewCharacterID);

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

// ------ 그 외 캐릭터 프로퍼티  ---------------
protected:
	//캐릭터의 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		FCharacterStatStruct CharacterStat;

	//캐릭터의 현재 상태
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Gameplay")
		FCharacterStateStruct CharacterState;

	//Character Item Inventory

	//Gamemode 약 참조
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//Gamemode 약 참조
		TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;

// ----- 타이머 관련 ---------------------------------
protected:
	UFUNCTION()
		virtual void ClearAllTimerHandle();

	//공격 간 딜레이 핸들
	UPROPERTY()
		FTimerHandle AtkIntervalHandle;

	UPROPERTY()
		FTimerHandle ReloadTimerHandle;

	UPROPERTY()
		TArray<FTimerHandle> SkillAnimPlayTimerHandleList;

	UPROPERTY()
		FTimerHandle KnockDownDelayTimerHandle;	

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