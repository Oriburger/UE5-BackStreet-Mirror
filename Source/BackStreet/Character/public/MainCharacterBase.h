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

// ------- ������Ʈ ----------
public:
	//�÷��̾� ���� ī�޶� ��
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		USpringArmComponent* CameraBoom;

	//�÷��̾��� ���� ī�޶�
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

	//�����⸦ �õ��Ѵ�.
	UFUNCTION()
		void Roll();

	//������ �� �뽬�� �õ��Ѵ�. AnimMontage�� Notify�� ���� ȣ��ȴ�.
	UFUNCTION(BlueprintCallable)
		void Dash();

	//ī�޶� Boom�� ���̸� ���̰ų� ���δ�.
	UFUNCTION()
		void ZoomIn(float Value);

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
		virtual void TrySkillAttack(ACharacterBase* Target) override;

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

	UFUNCTION()
		virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent
			, AController* EventInstigator, AActor* DamageCauser) override;

	//Rotation ���� ����� Ŀ�� ��ġ�� �Ѵ�
	UFUNCTION()
		void RotateToCursor();

	//1~4���� Ű�� ���� �������⸦ �����Ѵ�.
	UFUNCTION()
		void PickSubWeapon();

	UFUNCTION()
		TArray<AActor*> GetNearInteractionActorList();

// ------- �����Ƽ / ����� ---------------
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UAbilityManagerBase* GetAbilityManagerRef() { return AbilityManagerRef; }

public: 
	//����� ���¸� ����
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

//------- ��ų / �޺�----------
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

	//ĳ���Ͱ� �������� ���� ��, ���� Pulse ȿ���� ǥ�� �ؽ��� ȿ���� ����
	UFUNCTION()
		void SetFacialDamageEffect(bool NewState);

// -------- Asset ----------------
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Sound")
		class UAudioComponent* AudioComponent;


// ------- �� �� -----------
public:
	virtual void ClearAllTimerHandle() override;

private:
	UPROPERTY()
		class UAbilityManagerBase* AbilityManagerRef;

	//�÷��̾� ��Ʈ�ѷ� �� ����
	TWeakObjectPtr<class AMainCharacterController> PlayerControllerRef;

	//���� ��, ���콺 Ŀ���� ��ġ�� ĳ���Ͱ� �ٶ󺸴� ������ �ʱ�ȭ�ϴ� Ÿ�̸�
	//�ʱ�ȭ �ÿ��� �ٽ� movement �������� ĳ������ Rotation Set 
	UPROPERTY()
		FTimerHandle RotationResetTimerHandle;

	//���� �ݺ� �۾� Ÿ�̸�
	UPROPERTY()
		FTimerHandle AttackLoopTimerHandle;

	//������ ������ Ÿ�̸�
	UPROPERTY()
		FTimerHandle RollTimerHandle;

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
