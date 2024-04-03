// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "CharacterBase.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputMappingContext* DefaultMappingContext;

	// Move Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* MoveAction;

	// Roll Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* RollAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* ThrowReadyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* ThrowAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* InvestigateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* SwitchWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* DropWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* PickSubWeaponAction;

	// Jump Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* JumpAction;

	// Look Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* LookAction;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* SprintAction;

	// Crouch Input Action
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	//	class UInputAction* CrouchAction;

public:
	UFUNCTION()
		void Move(const FInputActionValue& Value);

	// Called for looking input
	UFUNCTION()
		void Look(const FInputActionValue& Value);

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
		void ZoomIn(float Value);

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
		virtual void TrySkill() override;

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

//-------- Skill ---------------------
public:
	//Add Skill Gauge when attacking(E_Attack) Enemy.
	UFUNCTION(BlueprintCallable)
		void AddSkillGauge();

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
		class UAbilityManagerBase* AbilityManagerRef;

	//플레이어 컨트롤러 약 참조
	TWeakObjectPtr<class AMainCharacterController> PlayerControllerRef;

	//공격 시, 마우스 커서의 위치로 캐릭터가 바라보는 로직을 초기화하는 타이머
	//초기화 시에는 다시 movement 방향으로 캐릭터의 Rotation Set 
	UPROPERTY()
		FTimerHandle RotationResetTimerHandle;

	//공격 반복 작업 타이머
	UPROPERTY()
		FTimerHandle AttackLoopTimerHandle;

	//구르기 딜레이 타이머
	UPROPERTY()
		FTimerHandle RollTimerHandle;

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
