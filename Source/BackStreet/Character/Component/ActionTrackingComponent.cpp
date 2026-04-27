// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionTrackingComponent.h"
#include "WeaponComponentBase.h"
#include "../CharacterBase.h"

// Sets default values for this component's properties
UActionTrackingComponent::UActionTrackingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//====================================================================
//====== Basic Function ==============================================

// Called when the game starts
void UActionTrackingComponent::BeginPlay()
{
	Super::BeginPlay();

	//활성화 여부 판단
	if (GetOwner()->ActorHasTag("Player") || bForceEnableMonitorForEnemy)
		bIsEnabled = true;

	//캐릭터 레퍼런스 셋
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner()); 

	if (OwnerCharacterRef.IsValid())
	{
		WeaponComponentRef = OwnerCharacterRef.Get()->WeaponComponent;
	}

	//타이머 이벤트 바인딩
	BindMonitorEvent();
}

void UActionTrackingComponent::BindMonitorEvent()
{
	if (!bIsEnabled) return;
	if (!WeaponComponentRef.IsValid() || !OwnerCharacterRef.IsValid())
	{
		if (bShowDebugMessage)
		{
			UE_LOG(LogTemp, Error, TEXT("UActionTrackingComponent::BindMonitorEvent() -> Weapon / OwnerCharacter is not valid"));
		}
		return;
	}

	TMap<FName, TUniquePtr<FDelegateOnActionTrigger>>& ownerActionTriggerMap = OwnerCharacterRef.Get()->ActionTriggerDelegateMap;
	TArray<FName> actionTriggerNameList; ownerActionTriggerMap.GenerateKeyArray(actionTriggerNameList);
	for (FName& name : actionTriggerNameList)
	{
		if (bShowDebugMessage)
		{
			UE_LOG(LogTemp, Warning, TEXT("name ; %s"), *name.ToString());
		}
	}

	TArray<ECharacterActionType> actionInfoKeyList; ActionInfoMap.GenerateKeyArray(actionInfoKeyList);
	for (ECharacterActionType& actionInfoKey : actionInfoKeyList)
	{
		FActionInfo& actionInfo = ActionInfoMap[actionInfoKey];

		switch (actionInfo.ActionCauserType)
		{
		case EActionCauserType::E_Owner:
			actionInfo.StartTrigger = OwnerCharacterRef.Get()->GetActionTriggerDelegate(actionInfo.ActionType)->AddLambda([=]() {
				TryUpdateActionState(actionInfoKey, EActionState::E_InProgress);
			});
			if (!actionInfo.bIsInstantAction)
			{
				actionInfo.StartTrigger = OwnerCharacterRef.Get()->GetActionTriggerDelegate(actionInfo.ActionType, true)->AddLambda([=]() {
					TryUpdateActionState(actionInfoKey, EActionState::E_RecentlyFinished);
				});
			}
			break;
		case EActionCauserType::E_Weapon:
			break;
		}
	}
}

void UActionTrackingComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	bIsEnabled = true; 
	BindMonitorEvent();
}

void UActionTrackingComponent::Deactivate()
{
	Super::Deactivate();
	bIsEnabled = false;
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

// Called every frame
void UActionTrackingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(bShowDebugMessage) PrintDebugMessage(DeltaTime);
}

void UActionTrackingComponent::PrintDebugMessage(float DeltaTime)
{
	TArray<ECharacterActionType> actionInfoKeyList; ActionInfoMap.GenerateKeyArray(actionInfoKeyList); 
	int32 keyId = 0;
	for (ECharacterActionType& actionInfoKey : actionInfoKeyList)
	{
		FActionInfo& actionInfo = ActionInfoMap[actionInfoKey];
		FString state;
		switch (actionInfo.ActionState)
		{
		case EActionState::E_Ready:
			state = "E_Ready";
			break;
		case EActionState::E_InProgress:
			state = "E_InProgress";
			break;
		case EActionState::E_RecentlyFinished:
			state = "E_RecentlyFinished";
			break;
		default:
			state = "E_NONE";
			break;
		}
		float remainingTime = actionInfo.GetRemainingTimeToReady(GetWorld());
		FString str = FString::Printf(TEXT("[Action %s] / State : %s / Cooldown : %.2f"), *actionInfo.ActionName.ToString(), *state, remainingTime);
		GEngine->AddOnScreenDebugMessage(keyId++, DeltaTime, FColor::Yellow, str);
	}
	GEngine->AddOnScreenDebugMessage(keyId++, DeltaTime, FColor::Yellow, FString::Printf(TEXT("[Combo] / Type : %d / Count : %d"), (int32)CurrentComboType, CurrentComboCount));
	GEngine->AddOnScreenDebugMessage(keyId, DeltaTime, FColor::Yellow, FString("[------- Action Tracking Debug Message -------]"));
}

bool UActionTrackingComponent::GetIsActionReady(ECharacterActionType ActionType)
{
	return GetActionState(ActionType) == EActionState::E_Ready;
}

bool UActionTrackingComponent::GetIsActionInProgress(ECharacterActionType ActionType)
{
	return GetActionState(ActionType) == EActionState::E_InProgress;
}

bool UActionTrackingComponent::GetIsActionRecentlyFinished(ECharacterActionType ActionType)
{
	return GetActionState(ActionType) == EActionState::E_RecentlyFinished;
}

//======================================================================
//====== Monitor Function ==============================================

void UActionTrackingComponent::SetActionResetTimer(FTimerHandle& TimerHandle, float& TimeoutValue, bool& Target)
{
	FTimerDelegate timerDelegate;
	timerDelegate.BindUFunction(this, FName("ResetValue"), Target);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, timerDelegate, TimeoutValue, false);
}

void UActionTrackingComponent::TryUpdateActionState(ECharacterActionType ActionType, EActionState NewState)
{
	if (!ActionInfoMap.Contains(ActionType)) return;
	FActionInfo& actionInfo  = ActionInfoMap[ActionType];
	uint8 currentState = (uint8)actionInfo.ActionState;

	//상태 전환이 불가능한 경우 리턴
	bool result = actionInfo.TrySetActionState(NewState);
	FName actionName = actionInfo.ActionName;
	if (!result && bShowDebugMessage)
	{
		UE_LOG(LogTemp, Error, TEXT("UActionTrackingComponent::TryUpdateActionState(%s, %d) failed"), *actionName.ToString(), (int32)NewState);
	}

	//델리게이트 브로드캐스트
	FActionStateUpdateDelegate& actionStateUpdateDelegate = GetActionStateUpdateDelegate(ActionType);

	if (NewState != EActionState::E_None)// && currentState != (uint8)NewState)
	{
		actionStateUpdateDelegate.Broadcast(ActionType, currentState, (uint8)NewState);
		OnGlobalActionStateUpdated.Broadcast(ActionType, currentState, (uint8)NewState);
	}
	
	//자동 전환 조건에 따라 타이머를 지B정한다.
	if (NewState == EActionState::E_InProgress && actionInfo.bIsInstantAction)
	{
		SetAutoTransitionTimer(ActionType, EActionState::E_RecentlyFinished, actionInfo.AutoFinishDelayForInstanceAction);
	}
	if (NewState == EActionState::E_RecentlyFinished)
	{
		SetAutoTransitionTimer(ActionType, EActionState::E_Ready);
	}
}

void UActionTrackingComponent::SetAutoTransitionTimer(ECharacterActionType ActionType, EActionState NewState, float DelayValueOverride)
{
	if (!ActionInfoMap.Contains(ActionType))
	{
		if (bShowDebugMessage)
		{
			FString actionName = UEnum::GetValueAsString((ECharacterActionType)ActionType);
			UE_LOG(LogTemp, Error, TEXT("UActionTrackingComponent::SetAutoTransitionTimer / %s"), *actionName);
		}
		return; 
	}
	FTimerDelegate timerDelegate;
	FActionInfo& actionInfo = ActionInfoMap[ActionType];
	timerDelegate.BindUFunction(this, FName("TryUpdateActionState"), actionInfo.ActionType, NewState);
	const float delayValue = DelayValueOverride <= 0.0f ? actionInfo.CooldownTimeoutThreshold : DelayValueOverride;
	FTimerHandle& targetHandle = (NewState == EActionState::E_RecentlyFinished ? actionInfo.AutoFinishTimerHandle : actionInfo.ActionResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(targetHandle, timerDelegate, delayValue, false);
}

FActionStateUpdateDelegate& UActionTrackingComponent::GetActionStateUpdateDelegate(ECharacterActionType ActionType)
{
	if (ActionInfoMap.Contains(ActionType))
	{
		return ActionInfoMap[ActionType].OnActionStateUpdated;
	}
	else
	{
		if (bShowDebugMessage)
		{
			FString actionName = UEnum::GetValueAsString((ECharacterActionType)ActionType);
			UE_LOG(LogTemp, Error, TEXT("UActionTrackingComponent::GetActionStateUpdateDelegate / %s"), *actionName);
		}
	}
	return OnGlobalActionStateUpdated;
}

void UActionTrackingComponent::ResetComboCount()
{
	CurrentComboType = EComboType::E_Normal;
	CurrentComboCount = 0;
}

EActionState UActionTrackingComponent::GetActionState(ECharacterActionType ActionType)
{
	if (!ActionInfoMap.Contains(ActionType))
	{
		if (bShowDebugMessage)
		{
			FString actionName = UEnum::GetValueAsString((ECharacterActionType)ActionType);
			UE_LOG(LogTemp, Error, TEXT("UActionTrackingComponent::GetActionState - Action / %s / is not found"), *actionName);
		}
		return EActionState::E_None;
	}
	FActionInfo& actionInfo = ActionInfoMap[ActionType];
	return actionInfo.ActionState;
}
