// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../CharacterBase.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "MainCharacterBase.generated.h"

class UInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateTutorialAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateTutorialMove);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateTutorialZoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateTutorialRoll);

UCLASS()
class BACKSTREET_API AMainCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTutorialAttack OnAttack;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTutorialMove OnMove;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTutorialZoom OnZoom;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateTutorialRoll OnRoll;


//-------- Global -----------------
public:
	AMainCharacterBase();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
		virtual void InitAsset(int32 NewCharacterID) override;

	UFUNCTION(BlueprintImplementableEvent)
		void InitCombatUI();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
						, FVector NormalImpulse, const FHitResult& Hit);

// ------- ������Ʈ ----------
public:
	//�÷��̾� ���� ī�޶� ��
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		USpringArmComponent* CameraBoom;

	//�÷��̾��� ���� ī�޶�
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		UCameraComponent* FollowingCamera;
		
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		class UItemInventoryComponent* ItemInventory;

// ------- Throw Test -----------

	UFUNCTION()
		void ReadyToThrow();

	UFUNCTION()
		void Throw();

	UFUNCTION()
		void SetAimingMode(bool bNewState);

	UFUNCTION()
		void UpdateAimingState();

	UFUNCTION()
		FVector GetThrowDestination();

	UPROPERTY()
		FTimerHandle AimingTimerHandle;

	UPROPERTY()
		bool bIsAiming = false;

// ------- Character Input Action ------- 
public:
	// MappingContext
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
		class UInputMappingContext* DefaultMappingContext;

	//Input Action Infos
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
		FPlayerInputActionInfo InputActionInfo;

	//Targeting system
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
		class UInputAction* LockToTargetAction;


// ------- Character Action ------- 
public:
	UFUNCTION()
		void Move(const FInputActionValue& Value);

	// Called for looking input
	UFUNCTION()
		void Look(const FInputActionValue& Value);

	UFUNCTION()
		void StartJump(const FInputActionValue& Value);

	// Called for sprinting input
	UFUNCTION()
		void Sprint(const FInputActionValue& Value);

	UFUNCTION()
		void StopSprint(const FInputActionValue& Value);

	//�����⸦ �õ��Ѵ�.
	UFUNCTION()
		void Roll();

	//Add Impulse to movement when rolling
	UFUNCTION(BlueprintCallable)
		void Dash();

	//ī�޶� Boom�� ���̸� ���̰ų� ���δ�.
	UFUNCTION()
		void ZoomIn(const FInputActionValue& Value);

	//�ֺ��� ��ȣ�ۿ� ������ ���͸� �����Ѵ�.
	UFUNCTION()
		void TryInvestigate();

	//�ش� ���Ϳ� ��ȣ�ۿ��Ѵ�.
	UFUNCTION(BlueprintCallable)
		void Investigate(AActor* TargetActor);

	UFUNCTION()
		virtual void TryReload() override;

	UFUNCTION(BlueprintCallable)
		virtual void TryAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual void TryUpperAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual void TryDownwardAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual void TrySkill(ESkillType SkillType, int32 SkillID) override;

	UFUNCTION(BlueprintCallable)
		virtual void Attack() override;

	UFUNCTION(BlueprintCallable)
		virtual void StopAttack() override;

	UFUNCTION()
		virtual void Die() override;

	//Rotation ���� ����� �⺻ ����� Movement �������� �ǵ�����
	UFUNCTION(BlueprintCallable)
		void ResetRotationToMovement();

	UFUNCTION()
		virtual void SwitchToNextWeapon() override;

	UFUNCTION()
		virtual void DropWeapon() override;

	//Targeting system
	UFUNCTION()
		void LockToTarget(const FInputActionValue& Value);

	UFUNCTION()
		virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
			, AController* EventInstigator, AActor* DamageCauser) override;

	//Rotation ���� ����� Ŀ�� ��ġ�� �Ѵ�
	UFUNCTION(BlueprintCallable)
		void RotateToCursor();

	//1~4���� Ű�� ���� �������⸦ �����Ѵ�.
	UFUNCTION()
		void PickSubWeapon(const FInputActionValue& Value);

	UFUNCTION()
		TArray<AActor*> GetNearInteractionActorList();

private:
	UFUNCTION()
		void StopDashMovement();

//------- Movement Interpolation Event---------
public:
	UFUNCTION()
		void SetWalkSpeedWithInterp(float NewValue, float InterpSpeed = 1.0f, const bool bAutoReset = false);

	UFUNCTION()
		void SetFieldOfViewWithInterp(float NewValue, float InterpSpeed = 1.0f, const bool bAutoReset = false);

private:
	//interp function
	//it must not be called alone.
	//if you set the value of bAutoReset false, you have to call this function to reset to original value
	UFUNCTION()
		void UpdateWalkSpeed(const float TargetValue, const float InterpSpeed = 1.0f, const bool bAutoReset = false);

	//interp function
	//it must not be called alone.
	UFUNCTION()
		void UpdateFieldOfView(const float TargetValue, float InterpSpeed = 1.0f, const bool bAutoReset = false);

// ------- SaveData ���� --------------------
protected:
	UFUNCTION()
		void SetCharacterStatFromSaveData();

public:
	UPROPERTY(BlueprintReadOnly, Category = "SaveData")
		FSaveData SavedData;

// ------- �����Ƽ / ����� ---------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAbilityManagerBase* GetAbilityManagerRef() { return AbilityManagerRef; }

public: 
	//����� ���¸� ����
	virtual	bool TryAddNewDebuff(FDebuffInfoStruct DebuffInfo, AActor* Causer);

	UFUNCTION(BlueprintCallable)
		bool TryAddNewAbility(int32 NewAbilityID);

	UFUNCTION(BlueprintCallable)
		bool TryRemoveAbility(int32 NewAbilityID);
		
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsAbilityActive(int32 AbilityID);

// -------- Inventory --------------
public:
	UFUNCTION(BlueprintCallable)
		virtual bool PickWeapon(int32 NewWeaponID) override;

// -------- VFX --------------------
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraComponent* BuffNiagaraEmitter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|VFX")
		class UNiagaraComponent* DirectionNiagaraEmitter;

private:
	UPROPERTY()
		bool bIsWallThroughEffectActivated = false;

	UFUNCTION()
		void ActivateDebuffNiagara(uint8 DebuffType);

	UFUNCTION()
		void DeactivateBuffEffect();

	//ĳ���Ͱ� �������� ���� ��, ���� Pulse ȿ���� ǥ�� �ؽ��� ȿ���� ����
	UFUNCTION()
		void SetFacialDamageEffect();

	UFUNCTION()
		void ResetFacialDamageEffect();

// -------- Asset ----------------
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class UAudioComponent* AudioComponent;

// ------- �� �� -----------
public:
	virtual void ClearAllTimerHandle() override;

private:
	UPROPERTY()
		float TargetFieldOfView = 0.0f;

	UPROPERTY()
		class UAbilityManagerBase* AbilityManagerRef;

	UPROPERTY()
		FVector2D MovementInputValue;

	//�÷��̾� ��Ʈ�ѷ� �� ����
	TWeakObjectPtr<class AMainCharacterController> PlayerControllerRef;

// ----- Timer Handle ---------------------
private:
	//���� ��, ���콺 Ŀ���� ��ġ�� ĳ���Ͱ� �ٶ󺸴� ������ �ʱ�ȭ�ϴ� Ÿ�̸�
	//�ʱ�ȭ �ÿ��� �ٽ� movement �������� ĳ������ Rotation Set 
	UPROPERTY()
		FTimerHandle RotationResetTimerHandle;

	//������ ������ Ÿ�̸�
	UPROPERTY()
		FTimerHandle RollTimerHandle;

	//WalkSpeed Interpolate timer
	UPROPERTY()
		FTimerHandle WalkSpeedInterpTimerHandle;

	//Field Of View Interpolate timer
	UPROPERTY()
		FTimerHandle FOVInterpHandle;

	//������ �� �뽬 ������ �ڵ�
	UPROPERTY()
		FTimerHandle DashDelayTimerHandle;

	//���� ���̾ư��� ����Ʈ ���� Ÿ�̸�
	UPROPERTY()
		FTimerHandle BuffEffectResetTimerHandle;

	//ĳ���� �� ȿ�� (��Ƽ���� �� ����) ���� Ÿ�̸�
	UPROPERTY()
		FTimerHandle FacialEffectResetTimerHandle;
};
