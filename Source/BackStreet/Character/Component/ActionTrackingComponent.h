// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "ActionTrackingComponent.generated.h"

// 전투 관련 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackBeginDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FActionStateUpdateDelegate, ECharacterActionType, ActionType, uint8, PrevActionState, uint8, ActionState);

USTRUCT(BlueprintType)
struct FActionInfo
{
	GENERATED_BODY()

	//===================================================================
	//====== Basic / Config =============================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		FName ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		ECharacterActionType ActionType;

	// 액션의 Causer -> (Delegate Broadcast 주체, 이를 기반으로 바인딩을 진행)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		EActionCauserType ActionCauserType;
	
	FDelegateHandle StartTrigger;

	// 인스턴트 액션인지? (종료 트리거가 별도로 없는 액션)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		bool bIsInstantAction;

	//E_InProgress에서 자동으로 E_RecentlyFinished 로 전환되기까지 걸리는 시간
	//InstantAction에서만 적용이 된다. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (EditCondition = "bIsInstantAction"))
		float AutoFinishDelayForInstanceAction = 0.25f;
	
	FDelegateHandle EndTrigger;

	// 쿨다운 타임아웃 값 (Ready 로 넘어가는 시간)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float CooldownTimeoutThreshold = 0.5f;

	//===================================================================
	//====== State ======================================================
		// 현재 액션의 상태
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Action")
		EActionState ActionState;

	//===================================================================
	//====== Timer & Delegate ===========================================
	FTimerHandle ActionResetTimerHandle;
	FTimerHandle AutoFinishTimerHandle;
	FTimerHandle GetTimerHandle() { return ActionResetTimerHandle; }

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FActionStateUpdateDelegate OnActionStateUpdated;
	
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

		//액션 상태간 전환할 수 있는 그래프를 정의
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
					, bIsInstantAction(false), ActionState(EActionState::E_Ready) {}
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

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FActionStateUpdateDelegate OnGlobalActionStateUpdated;

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsActionReady(ECharacterActionType ActionType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsActionInProgress(ECharacterActionType ActionType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsActionRecentlyFinished(ECharacterActionType ActionType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		EActionState GetActionState(ECharacterActionType ActionType);

//타이머 이벤트 bind 전용 함수
private:
	void SetActionResetTimer(FTimerHandle& TimerHandle, float& TimeoutValue, bool& Target);
	bool bIsEnabled = false;

// delegate bind 전용 내부 함수
protected:
	UFUNCTION()
		void TryUpdateActionState(ECharacterActionType ActionType, EActionState NewState);

	UFUNCTION()
		void SetAutoTransitionTimer(ECharacterActionType ActionType, EActionState NewState, float DelayValueOverride = -1.0f);

//외부에서 델리게이트에 접근할 수 있도록 함
public:
	FActionStateUpdateDelegate& GetActionStateUpdateDelegate(ECharacterActionType ActionType);

//======================================================================
//====== Monitor Property ==============================================
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		TMap<ECharacterActionType, FActionInfo> ActionInfoMap;

//=====================================================================
//====== Monitor Config ===============================================
protected:
	//디버그용) 적들에게도 모니터링 타이머를 적용할지 유무 판단
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Debug")
		bool bForceEnableMonitorForEnemy = false;
	
	//디버그 메시지를 띄울지 여부 결정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Debug")
		bool bShowDebugMessage = true;

	//최대 콤보 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (UIMin = 0.0f, UIMax = 1.5f))
		int32 MaxComboCount = 9999;
		
//=====================================================================
//====== etc ==========================================================
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
		EComboType CurrentComboType = EComboType::E_Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
		int32 CurrentComboCount = 0;

public:
	UFUNCTION(BlueprintCallable)
		int32 UpdateComboCount() { return CurrentComboCount += 1; }

	UFUNCTION(BlueprintCallable)
		void ResetComboCount();

protected:
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class UWeaponComponentBase> WeaponComponentRef;
};
