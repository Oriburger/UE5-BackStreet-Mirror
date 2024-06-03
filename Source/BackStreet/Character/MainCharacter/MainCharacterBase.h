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

// ------- 컴포넌트 ----------
public:
	//플레이어 메인 카메라 붐
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		USpringArmComponent* CameraBoom;

	//플레이어의 메인 카메라
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

	//구르기를 시도한다.
	UFUNCTION()
		void Roll();

	//Add Impulse to movement when rolling
	UFUNCTION(BlueprintCallable)
		void Dash();

	//카메라 Boom의 길이를 늘이거나 줄인다.
	UFUNCTION()
		void ZoomIn(const FInputActionValue& Value);

	//주변에 상호작용 가능한 액터를 조사한다.
	UFUNCTION()
		void TryInvestigate();

	//해당 액터와 상호작용한다.
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

	//Rotation 조절 방식을 기본 방식인 Movement 방향으로 되돌린다
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

	//Rotation 조절 방식을 커서 위치로 한다
	UFUNCTION(BlueprintCallable)
		void RotateToCursor();

	//1~4번의 키를 눌러 보조무기를 장착한다.
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

// ------- SaveData 관련 --------------------
protected:
	UFUNCTION()
		void SetCharacterStatFromSaveData();

public:
	UPROPERTY(BlueprintReadOnly, Category = "SaveData")
		FSaveData SavedData;

// ------- 어빌리티 / 디버프 ---------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAbilityManagerBase* GetAbilityManagerRef() { return AbilityManagerRef; }

public: 
	//디버프 상태를 지정
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

	//캐릭터가 데미지를 입을 시, 빨간 Pulse 효과와 표정 텍스쳐 효과를 적용
	UFUNCTION()
		void SetFacialDamageEffect();

	UFUNCTION()
		void ResetFacialDamageEffect();

// -------- Asset ----------------
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class UAudioComponent* AudioComponent;

// ------- 그 외 -----------
public:
	virtual void ClearAllTimerHandle() override;

private:
	UPROPERTY()
		float TargetFieldOfView = 0.0f;

	UPROPERTY()
		class UAbilityManagerBase* AbilityManagerRef;

	UPROPERTY()
		FVector2D MovementInputValue;

	//플레이어 컨트롤러 약 참조
	TWeakObjectPtr<class AMainCharacterController> PlayerControllerRef;

// ----- Timer Handle ---------------------
private:
	//공격 시, 마우스 커서의 위치로 캐릭터가 바라보는 로직을 초기화하는 타이머
	//초기화 시에는 다시 movement 방향으로 캐릭터의 Rotation Set 
	UPROPERTY()
		FTimerHandle RotationResetTimerHandle;

	//구르기 딜레이 타이머
	UPROPERTY()
		FTimerHandle RollTimerHandle;

	//WalkSpeed Interpolate timer
	UPROPERTY()
		FTimerHandle WalkSpeedInterpTimerHandle;

	//Field Of View Interpolate timer
	UPROPERTY()
		FTimerHandle FOVInterpHandle;

	//구르기 내 대쉬 딜레이 핸들
	UPROPERTY()
		FTimerHandle DashDelayTimerHandle;

	//버프 나이아가라 이펙트 리셋 타이머
	UPROPERTY()
		FTimerHandle BuffEffectResetTimerHandle;

	//캐릭터 얼굴 효과 (머티리얼 값 변경) 리셋 타이머
	UPROPERTY()
		FTimerHandle FacialEffectResetTimerHandle;
};
