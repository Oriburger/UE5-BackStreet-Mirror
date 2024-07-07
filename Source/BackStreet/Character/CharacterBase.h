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

	//Weapon�� �ı��Ǿ����� ȣ���� �̺�Ʈ
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UWeaponComponentBase* WeaponComponent;

	UPROPERTY(VisibleAnywhere)
		class UDebuffManagerComponent* DebuffManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UTargetingManagerComponent* TargetingManagerComponent;

// ------- Character Action �⺻ ------- 
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		int32 CharacterID;

	//Input�� Binding �Ǿ� ������ �õ� (AnimMontage�� ȣ��)
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

	///Input�� Binding �Ǿ� ��ų������ �õ� (AnimMontage�� ȣ��)
	virtual void TrySkill(ESkillType SkillType, int32 SkillID);

	bool CheckCanTrySkill(int32 SkillID, FOwnerSkillInfoStruct* SkillInfo);

	//AnimNotify�� Binding �Ǿ� ���� ������ ����
	virtual void Attack();

	virtual void StopAttack();

	virtual void TryReload();

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
		, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Die();

	//�÷��̾ ���� �ش� Action�� �����ϰ� �ִ��� ��ȯ
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsActionActive(ECharacterActionType Type) { return CharacterState.CharacterActionState == Type; }

	//�÷��̾��� ActionState�� Idle�� ��ȯ�Ѵ�.
	UFUNCTION(BlueprintCallable)
		void ResetActionState(bool bForceReset = false);

	//Set Player's Action State
	UFUNCTION(BlueprintCallable)
		void SetActionState(ECharacterActionType Type);

	//�÷��̾ ü���� ȸ���� (��ȸ��)
	UFUNCTION(BlueprintCallable)
		void TakeHeal(float HealAmount, bool bIsTimerEvent = false, uint8 BuffDebuffType = 0);

	//����� �������� ���� (��ȸ��)
	UFUNCTION(BlueprintCallable)
		float TakeDebuffDamage(ECharacterDebuffType DebuffType, float DamageAmount, AActor* Causer);

	UFUNCTION(BlueprintCallable)
		void ApplyKnockBack(AActor* Target, float Strength);

	//����Action ������ Interval�� �����ϴ� Ÿ�̸Ӹ� ����
	UFUNCTION()
		void ResetAtkIntervalTimer();

	//Update Character's world Location using interporlation
	UFUNCTION(BlueprintCallable)
		void SetLocationWithInterp(FVector NewValue, float InterpSpeed = 1.0f, const bool bAutoReset = false);

	UFUNCTION(BlueprintCallable)
		void ResetLocationInterpTimer();
	
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
	//
	//UFUNCTION(BlueprintCallable);
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

	//Knock down with 
	virtual void KnockDown();

// ------- Character Stat/State ------------------------------
public:
	//ĳ������ ���� ������ �ʱ�ȭ
	UFUNCTION()
		void InitCharacterState();

	//ĳ������ ����� ������ ������Ʈ
	UFUNCTION(BlueprintCallable)
		virtual	bool TryAddNewDebuff(FDebuffInfoStruct DebuffInfo, AActor* Causer);

	//������� Ȱ��ȭ �Ǿ��ִ��� ��ȯ
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetDebuffIsActive(ECharacterDebuffType DebuffType);

	//Update Character's stat and state
	UFUNCTION(BlueprintCallable)
		void UpdateCharacterStatAndState(FCharacterStatStruct NewStat, FCharacterStateStruct NewState);

	//ĳ������ ������ ������Ʈ
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

// ------ ���� ���� -------------------------------------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool PickWeapon(int32 NewWeaponID);
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

	//�� ������ ���̺� (���� ���� ����)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Asset")
		UDataTable* AssetDataInfoTable;

public:
	UPROPERTY(BlueprintReadOnly)
		TArray<USoundCue*> FootStepSoundList;

// ------ �� �� ĳ���� ������Ƽ  ---------------
protected:
	//ĳ������ ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		FCharacterStatStruct CharacterStat;

	//ĳ������ ���� ����
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Gameplay")
		FCharacterStateStruct CharacterState;

	//Character Item Inventory

	//Gamemode �� ����
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	//Gamemode �� ����
		TWeakObjectPtr<class UAssetManagerBase> AssetManagerBaseRef;

// ----- Ÿ�̸� ���� ---------------------------------
protected:
	UFUNCTION()
		virtual void ClearAllTimerHandle();

	//���� �� ������ �ڵ�
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