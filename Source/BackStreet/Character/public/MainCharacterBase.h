// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "CharacterBase.h"
#include "GameFramework/Character.h"
#include "MainCharacterBase.generated.h"

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

// ------- Character Action ------- 
public:
	UFUNCTION()
		void MoveForward(float Value);

	UFUNCTION()
		void MoveRight(float Value);

	//구르기를 시도한다.
	UFUNCTION()
		void Roll();

	//구르기 시 대쉬를 시도한다. AnimMontage의 Notify에 의해 호출된다.
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
		virtual void TrySkillAttack(ACharacterBase* Target) override;

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
	UFUNCTION()
		void RotateToCursor();

	//1~4번의 키를 눌러 보조무기를 장착한다.
	UFUNCTION()
		void PickSubWeapon();

	UFUNCTION()
		TArray<AActor*> GetNearInteractionActorList();

// ------- 어빌리티 / 디버프 ---------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAbilityManagerBase* GetAbilityManagerRef() { return AbilityManagerRef; }

public: 
	//디버프 상태를 지정
	virtual	bool TryAddNewDebuff(ECharacterDebuffType NewDebuffType, AActor* Causer = nullptr, float TotalTime = 0.0f, float Value = 0.0f);

	UFUNCTION(BlueprintCallable)
		bool TryAddNewAbility(const ECharacterAbilityType NewAbilityType);

	UFUNCTION(BlueprintCallable)
		bool TryRemoveAbility(const ECharacterAbilityType TargetAbilityType);
		
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsAbilityActive(const ECharacterAbilityType TargetAbilityType);

// -------- Inventory --------------
public:
	UFUNCTION(BlueprintCallable)
		virtual bool PickWeapon(int32 NewWeaponID) override;

//------- 스킬 / 콤보----------
public:
	UFUNCTION(BlueprintCallable)
		void AddSkillGauge();

// -------- VFX -----------
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

	UFUNCTION()
		void UpdateWallThroughEffect();

	//캐릭터가 데미지를 입을 시, 빨간 Pulse 효과와 표정 텍스쳐 효과를 적용
	UFUNCTION()
		void SetFacialDamageEffect(bool NewState);

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
