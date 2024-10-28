// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "ActionTrackingComponent.generated.h"

// 전투 관련 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackBeginDelegate);

UENUM(BlueprintType)
enum class EActionState : uint8
{
	E_None				UMETA(DisplayName = "None"),				 // Error Handling
	E_Ready				UMETA(DisplayName = "Ready"),                // 준비 상태
	E_InProgress		UMETA(DisplayName = "In Progress"),			 // 액션 진행 중
	E_RecentlyFinished	UMETA(DisplayName = "Recently Finished")	 // 최근 종료
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

	// 액션의 Causer -> (Delegate Broadcast 주체, 이를 기반으로 바인딩을 진행)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
		EActionCauserType ActionCauserType;

	// 액션의 시작 트리거 델리게이트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Trigger")
		FName StartTriggerName;
		FDelegateHandle StartTrigger;

	// 인스턴트 액션인지? (종료 트리거가 별도로 없는 액션)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		bool bIsInstantAction;

	//E_InProgress에서 자동으로 E_RecentlyFinished 로 전환되기까지 걸리는 시간
	//InstantAction에서만 적용이 된다. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (EditCondition = "bIsInstantAction"))
		float AutoFinishDelayForInstanceAction = 0.25f;

	// 액션의 종료 트리거 델리게이트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Trigger", meta = (EditCondition = "!bIsInstantAction"))
		FName EndTriggerName;
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

//타이머 이벤트 bind 전용 함수
private:
	void SetActionResetTimer(FTimerHandle& TimerHandle, float& TimeoutValue, bool& Target);
	bool bIsEnabled = false;

// delegate bind 전용 내부 함수
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
	//디버그용) 적들에게도 모니터링 타이머를 적용할지 유무 판단
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Debug")
		bool ForceEnableMonitorForEnemy = false;

	//최대 콤보 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (UIMin = 0.0f, UIMax = 1.5f))
		int32 MaxComboCount = 9999;
		
//=====================================================================
//====== etc ==========================================================
protected:
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<class UWeaponComponentBase> WeaponComponentRef;
};
