// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "ActionTrackingComponent.generated.h"

// ���� ���� ��������Ʈ ����
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackBeginDelegate);

UENUM(BlueprintType)
enum class EActionState : uint8
{
	E_None				UMETA(DisplayName = "None"),				 // Error Handling
	E_Ready				UMETA(DisplayName = "Ready"),                // �غ� ����
	E_InProgress		UMETA(DisplayName = "In Progress"),			 // �׼� ���� ��
	E_RecentlyFinished	UMETA(DisplayName = "Recently Finished")	 // �ֱ� ����
};

UENUM(BlueprintType)
enum class EActionCauserType : uint8
{
	E_None				UMETA(DisplayName = "None"),		//Error Handling		
	E_Owner				UMETA(DisplayName = "Owner"),		//Owner Character
	E_Weapon			UMETA(DisplayName = "Weapon"),		//Owner's WeaponComponent
	E_Enemy				UMETA(DisplayName = "Enemy")		//Enemy Character
};

USTRUCT(BlueprintType)
struct FActionInfo
{
	GENERATED_BODY()

	//===================================================================
	//====== Basic / Config =============================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		FName ActionName;

	// �׼��� Causer -> (Delegate Broadcast ��ü, �̸� ������� ���ε��� ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		EActionCauserType ActionCauserType;

	// �׼��� ���� Ʈ���� ��������Ʈ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Trigger")
		FName StartTriggerName;
		FDelegateHandle StartTrigger;

	// �ν���Ʈ �׼�����? (���� Ʈ���Ű� ������ ���� �׼�)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		bool bIsInstantAction;

	//E_InProgress���� �ڵ����� E_RecentlyFinished �� ��ȯ�Ǳ���� �ɸ��� �ð�
	//InstantAction������ ������ �ȴ�. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (EditCondition = "bIsInstantAction"))
		float AutoFinishDelayForInstanceAction = 0.25f;

	// �׼��� ���� Ʈ���� ��������Ʈ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Trigger", meta = (EditCondition = "!bIsInstantAction"))
		FName EndTriggerName;
		FDelegateHandle EndTrigger;

	// ��ٿ� Ÿ�Ӿƿ� �� (Ready �� �Ѿ�� �ð�)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float CooldownTimeoutThreshold = 0.5f;

	//===================================================================
	//====== State ======================================================
		// ���� �׼��� ����
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Action")
		EActionState ActionState;

	//===================================================================
	//====== Timer ======================================================
	FTimerHandle ActionResetTimerHandle;
	FTimerHandle AutoFinishTimerHandle;
	FTimerHandle GetTimerHandle() { return ActionResetTimerHandle; }

	//===================================================================
	//====== Function ===================================================
	bool GetActionIsReady() { return ActionState == EActionState::E_Ready; }
	bool GetActionIsActive() { return ActionState == EActionState::E_InProgress; }
	bool GetActionIsRecentlyEnd() { return ActionState == EActionState::E_RecentlyFinished; }
	float GetRemainingTimeToReady(const UObject* WorldContextObject)
	{
		const UWorld* world = Cast<UWorld>(WorldContextObject);
		if (IsValid(world))
		{
			return world->GetTimerManager().GetTimerRemaining(ActionResetTimerHandle);
		}
		return -1.0f;
	}
	bool TrySetActionState(EActionState NewState)
	{
		if (ActionState == NewState) return true;
		if (NewState == EActionState::E_None) return false;

		//�׼� ���°� ��ȯ�� �� �ִ� �׷����� ����
		switch (NewState)
		{
		case EActionState::E_Ready:
			if (ActionState == EActionState::E_InProgress)
				return false;
			break;
		case EActionState::E_RecentlyFinished:
			if (ActionState == EActionState::E_Ready)
				return false;
			break;
		}
		ActionState = NewState;
		return true;
	}

	bool operator==(const FActionInfo& Other) const { return Other.ActionName == ActionName; }

	FActionInfo() : ActionName(""), ActionCauserType(EActionCauserType::E_None)
					, StartTriggerName(FName("")), bIsInstantAction(false), EndTriggerName(FName(""))
					, ActionState(EActionState::E_Ready) {}
};

UCLASS(Blueprintable)
class BACKSTREET_API UActionTrackingComponent : public UActorComponent
{
	GENERATED_BODY()
	
//===================================================================
//====== Common, Delegate ===========================================
public:	
	// Sets default values for this component's properties
	UActionTrackingComponent();

//====================================================================
//====== Basic Function ==============================================
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void BindMonitorEvent(); 

	virtual void Activate(bool bReset) override;

	virtual void Deactivate() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void PrintDebugMessage(float DeltaTime);

//======================================================================
//====== Monitor Function ==============================================
public:	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsEnabled() { return bIsEnabled; }

//Ÿ�̸� �̺�Ʈ bind ���� �Լ�
private:
	void SetActionResetTimer(FTimerHandle& TimerHandle, float& TimeoutValue, bool& Target);
	bool bIsEnabled = false;

// delegate bind ���� ���� �Լ�
protected:
	UFUNCTION()
		void TryUpdateActionState(FName ActionName, EActionState NewState);

	UFUNCTION()
		void SetAutoTransitionTimer(FName ActionName, EActionState NewState, float DelayValueOverride = -1.0f);

//======================================================================
//====== Monitor Property ==============================================
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TMap<FName, FActionInfo> ActionInfoMap;

//=====================================================================
//====== Monitor Config ===============================================
protected:
	//����׿�) ���鿡�Ե� ����͸� Ÿ�̸Ӹ� �������� ���� �Ǵ�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Debug")
		bool ForceEnableMonitorForEnemy = false;

	//�ִ� �޺� ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (UIMin = 0.0f, UIMax = 1.5f))
		int32 MaxComboCount = 9999;
		
//=====================================================================
//====== etc ==========================================================
protected:
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class UWeaponComponentBase> WeaponComponentRef;
};
