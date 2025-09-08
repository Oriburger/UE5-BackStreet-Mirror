// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../CharacterBase.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "MainCharacterBase.generated.h"

class UInputComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateZoomBegin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateZoomEnd);

UCLASS()
class BACKSTREET_API AMainCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateZoomBegin OnZoomBegin;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateZoomBegin OnZoomEnd;

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

	//플레이어 메인 카메라 붐 아래에 부드러운 카메라 렉을 위한 서브붐
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		USpringArmComponent* SubCameraBoom;

	//플레이어의 메인 카메라
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		UCameraComponent* FollowingCamera;

	//Player SpirngArm for RangedWeapon Aim
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		USpringArmComponent* RangedAimBoom;

	//Player SpirngArm for Targeting Mode
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		USpringArmComponent* TargetingModeBoom;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		class UItemInventoryComponent* ItemInventory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UAbilityManagerComponent* AbilityManagerComponent; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UTargetingManagerComponent* TargetingManagerComponent;

protected:
	//스프링암 길이에 따라 캐릭터 폰 비저빌리티 업데이트
	UFUNCTION()
		void UpdateVisibilityByDistance();

// ------- Throw Test -----------
public:
	UFUNCTION(BlueprintCallable)
		void SwitchWeapon(bool bSwitchToSubWeapon, bool bForceApply = false);
	
	UFUNCTION()
		void ZoomIn();

	UFUNCTION()
		void ZoomOut();

	UFUNCTION()
		void TryShoot();

	UFUNCTION(BlueprintCallable)
		void TryShootBySubCombo(FRotator ShootRotation, FVector ShootLocation);

	UFUNCTION()
		void SetAimingMode(bool bNewState);
			
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FRotator GetAimingRotation(FVector BeginLocation);

// ------- Character Input Action ------- 
public:
	// MappingContext
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
		class UInputMappingContext* DefaultMappingContext;

	//Input Action Infos
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
		FPlayerInputActionInfo InputActionInfo;

	// Move Key input check
	UPROPERTY(BlueprintReadOnly)
		bool bMoveKeyDown = false;

	// Movement Input Value
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector2D GetCurrentMovementInputValue() { return MovementInputValue; }

protected:
	UFUNCTION()
		void ResetMovementInputValue();

	UPROPERTY()
		FVector2D MovementInputValue;

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

	//주변에 상호작용 가능한 액터를 조사한다.
	UFUNCTION()
		void TryInvestigate();

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

	UFUNCTION()
		virtual void StandUp() override;

	//Targeting system
	UFUNCTION()
		void ToggleTargetingMode(const FInputActionValue& Value);

	UFUNCTION()
		void SnapToCharacter(AActor* Target);

	UFUNCTION()
		virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
			, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
		class UInteractiveCollisionComponent* GetNearInteractionComponent();

protected:
	UFUNCTION()
		virtual TArray<UAnimMontage*> GetTargetMeleeAnimMontageList() override;

private:
	UFUNCTION()
		void StopDashMovement();

	TWeakObjectPtr<class UInteractiveCollisionComponent> InteractionCandidateRef;

//------- Camera Rotation Support Event ------ 

public:
	UFUNCTION()
		void SetCameraVerticalAlignmentWithInterp(float TargetPitch, const float InterpSpeed);

	UFUNCTION()
		void ResetCameraRotation();

	UFUNCTION()
		void ResetCameraBoom();

protected:
	UFUNCTION()
		void UpdateMainCameraBoom(class USpringArmComponent* TargetBoom);

	UFUNCTION()
		void SetAutomaticRotateModeTimer();

	UFUNCTION()
		void SetAutomaticRotateMode();	

	UFUNCTION()
		void SetManualRotateMode();

	//If character run forward to camera using down move input when automatic rotate mode is activated,
	// this function will be called then remove the camera shake by setting camera boom's rotate lag speed to zero.
	UFUNCTION()
		void SetRotationLagSpeed(FVector2D ModeInput);

	UFUNCTION()
		void OnTargetingStateUpdated(bool bIsActivated, APawn* Target);

protected:
	UFUNCTION()
		void UpdateCameraPitch(float TargetPitch, float InterpSpeed = 1.0f);	

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Camera|AutoRotateSupport")
		bool bIsManualMode = false;

	UPROPERTY(EditAnywhere, Category = "Camera|AutoRotateSupport")
		float FaceToFaceLagSpeed = 0.001;
	
	UPROPERTY(EditAnywhere, Category = "Camera|AutoRotateSupport")
		float NoramlLagSpeed = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Camera|AutoRotateSupport")
		float AutomaticModeSwitchTime = 3.0f;

//------- Movement Interpolation Event---------
public:
	UFUNCTION()
		void SetWalkSpeedWithInterp(float NewValue, float InterpSpeed = 1.0f, const bool bAutoReset = false);

	UFUNCTION(BlueprintCallable)
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

//-------- EasyUI LoadSettings --------------
protected:
	UPROPERTY(BlueprintReadWrite)
		bool bHoldToSprint;

	UPROPERTY(BlueprintReadWrite)
		bool bInvertYAxis;

	UPROPERTY(BlueprintReadWrite)
		bool bInvertXAxis;

	UPROPERTY(BlueprintReadWrite)
		float CameraSensivity;

// ------- SaveData 관련 --------------------
public:
	UFUNCTION(BlueprintCallable)
		void UpdateDamageAmount(float NewDamage = -1.0f, float NewSubDamage = -1.0f);

protected:
	UFUNCTION()
		void SetCharacterStatFromSaveData();

// ------- 어빌리티 / 디버프 ---------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAbilityManagerComponent* GetAbilityManagerRef() { return AbilityManagerComponent; }

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
	virtual bool EquipWeapon(int32 NewWeaponID);

private:
	UPROPERTY()
		bool bIsWallThroughEffectActivated = false;

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

	//플레이어 컨트롤러 약 참조
	TWeakObjectPtr<class AMainCharacterController> PlayerControllerRef;

// ----- Timer Handle ---------------------
private:
	//공격 시, 마우스 커서의 위치로 캐릭터가 바라보는 로직을 초기화하는 타이머
	//초기화 시에는 다시 movement 방향으로 캐릭터의 Rotation Set 
	FTimerHandle RotationResetTimerHandle;

	FTimerHandle CameraRotationAlignmentHandle;
	
	//구르기 딜레이 타이머
	FTimerHandle RollDelayTimerHandle;

	//연속 점프 공격 시 스턴 주기위함
	FTimerHandle JumpAttackDebuffTimerHandle;
	int32 JumpAttackCount = 0;

	//WalkSpeed Interpolate timer
	FTimerHandle WalkSpeedInterpTimerHandle;

	//Field Of View Interpolate timer
	FTimerHandle FOVInterpHandle;

	//구르기 내 대쉬 딜레이 핸들
	FTimerHandle DashDelayTimerHandle;

	//버프 나이아가라 이펙트 리셋 타이머
	FTimerHandle BuffEffectResetTimerHandle;

	//캐릭터 얼굴 효과 (머티리얼 값 변경) 리셋 타이머
	FTimerHandle FacialEffectResetTimerHandle;

	FTimerHandle SwitchCameraRotateModeTimerHandle;
};
