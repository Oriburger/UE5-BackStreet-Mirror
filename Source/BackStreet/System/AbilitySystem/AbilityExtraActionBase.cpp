// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityExtraActionBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/BattleApplicationManager.h"
#include "../../Character/Component/ActionTrackingComponent.h"

// Sets default values
AAbilityExtraActionBase::AAbilityExtraActionBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AAbilityExtraActionBase::BeginPlay()
{
    Super::BeginPlay();
}

void AAbilityExtraActionBase::OnTriggerActionStateUpdated(ECharacterActionType ActionType, uint8 PrevActionState, uint8 ActionState)
{
	if (ActionType != LinkedAbilityInfo.TriggerActionType)
    {
		FString actionName = UEnum::GetValueAsString(ActionType);
		UE_LOG(LogAbility, Error, TEXT("AAbilityExtraActionBase::OnTriggerActionStateUpdated - ActionName mismatch: %s != %s"), *actionName, *LinkedAbilityInfo.GetActionTriggerName());
        Destroy();
    }

	//트리거 액션 상태가 일치하지 않으면 무시
    if (LinkedAbilityInfo.TriggerActionState != static_cast<EActionState>(ActionState)) return;

	//트리거 액션 상태가 일치하면 추가 액션 
    ExecuteExtraAction();
}

void AAbilityExtraActionBase::OnAbilityInfoUpdated(int32 AbilityID, FAbilityInfoStruct NewAbilityInfo)
{
    if (AbilityID != LinkedAbilityInfo.AbilityId)
    {
        UE_LOG(LogAbility, Error, TEXT("AAbilityExtraActionBase::OnAbilityInfoUpdated - AbilityID mismatch: %d != %d"), AbilityID, LinkedAbilityInfo.AbilityId);
        return;
    }
    UnbindFromActionTrackingEvents(LinkedAbilityInfo);
	LinkedAbilityInfo = NewAbilityInfo;
	BindtoActionTrackingEvents(LinkedAbilityInfo);
}

void AAbilityExtraActionBase::InitializeExtraAction(AMainCharacterBase* NewOwnerCharacter, UAbilityManagerComponent* NewAbilityManagerComponent, FAbilityInfoStruct NewLinkedAbilityInfo)
{
    if(!IsValid(NewOwnerCharacter) || !IsValid(NewAbilityManagerComponent))
    {
        UE_LOG(LogAbility, Error, TEXT("AAbilityExtraActionBase::InitializeExtraAction - Invalid OwnerCharacter or BattleApplicationManager"));
        return;
	}
    if(NewLinkedAbilityInfo.AbilityId <= 0 || NewLinkedAbilityInfo.AbilityName == NAME_None)
    {
		UE_LOG(LogAbility, Error, TEXT("AAbilityExtraActionBase::InitializeExtraAction - Invalid Ability / id : %d, name : %s")
                                    , NewLinkedAbilityInfo.AbilityId, *NewLinkedAbilityInfo.AbilityName.ToString());
        return;
	}

	LinkedAbilityInfo = NewLinkedAbilityInfo;
    OwnerCharacterRef = NewOwnerCharacter;
    AbilityManagerComponentRef = NewAbilityManagerComponent;
    ActionTrackingComponentRef = OwnerCharacterRef.IsValid() ? OwnerCharacterRef->ActionTrackingComponent : nullptr;

    if (!OwnerCharacterRef.IsValid() || !ActionTrackingComponentRef.IsValid())
    {
		UE_LOG(LogAbility, Error, TEXT("AAbilityExtraActionBase::InitializeExtraAction - Invalid Error (OwnerCharacterRef? : %d, ActionTrackingComponentRef? : %d)")
                                    , OwnerCharacterRef.IsValid() ? 1 : 0
			                        , ActionTrackingComponentRef.IsValid() ? 1 : 0);
        Destroy();
        return;
    }

	//자동 실행 여부에 따라 처리 분기
    if (LinkedAbilityInfo.bIsAutoTrigger)
    {
		UE_LOG(LogAbility, Log, TEXT("AAbilityExtraActionBase::InitializeExtraAction - Auto Triggering Extra Action for Ability: %s"), *LinkedAbilityInfo.AbilityName.ToString());
        //        ExecuteExtraAction();
    }
    else
    {
		UE_LOG(LogAbility, Log, TEXT("AAbilityExtraActionBase::InitializeExtraAction - Binding to Action Tracking Events for Ability: %s"), *LinkedAbilityInfo.AbilityName.ToString());
        BindtoActionTrackingEvents(LinkedAbilityInfo);
    }
}

void AAbilityExtraActionBase::ExecuteExtraAction_Implementation()
{
	bIsExecuting = true;
	OnExtraActionTriggered.Broadcast(LinkedAbilityInfo.AbilityId, this);
	UE_LOG(LogAbility, Log, TEXT("AAbilityExtraActionBase::ExecuteExtraAction - Executing Extra Action for Ability: %s"), *LinkedAbilityInfo.AbilityName.ToString());
}

void AAbilityExtraActionBase::StopExtraAction_Implementation()
{
    bIsExecuting = false;
    UE_LOG(LogAbility, Log, TEXT("AAbilityExtraActionBase::StopExtraAction - Stopping Extra Action for Ability: %s"), *LinkedAbilityInfo.AbilityName.ToString());
    if (LinkedAbilityInfo.bIsOneTimeUse)
    {
        GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
		OnExtraActionLifeSpanEnded.Broadcast(LinkedAbilityInfo.AbilityId, this);
        return;
    }
    OnExtraActionCompleted.Broadcast(LinkedAbilityInfo.AbilityId, this);
}

bool AAbilityExtraActionBase::GetTriggerValidity()
{
    if (!ActionTrackingComponentRef.IsValid() || !OwnerCharacterRef.IsValid())
    {
		UE_LOG(LogAbility, Error, TEXT("AAbilityExtraActionBase::GetTriggerValidity - Invalid ActionTrackingComponentRef %d, OwnerCharacterRef %d"), ActionTrackingComponentRef.IsValid() ? 1 : 0, OwnerCharacterRef.IsValid() ? 1 : 0);
        return false;
    }
    FString triggerName = UEnum::GetValueAsString(LinkedAbilityInfo.TriggerActionType);

    if (LinkedAbilityInfo.bIsAutoTrigger) return true;
    else if (triggerName.Contains("Attack"))
    {
        if (LinkedAbilityInfo.TriggerComboIdxList.IsEmpty()) return false;
        int32 maxComboIdx = triggerName.Contains("Dash") ? (int32)OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_MaxDashComboIdx)
                            : (triggerName.Contains("Jump") ? (int32)OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_MaxAerialComboIdx)
                            : (int32)OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_MaxNormalComboIdx));
		int32 currComboIdx = ActionTrackingComponentRef.Get()->CurrentComboCount % maxComboIdx;
        return LinkedAbilityInfo.TriggerComboIdxList.Contains(currComboIdx);
    }
	else if (LinkedAbilityInfo.TriggerActionType == ECharacterActionType::COUNT || LinkedAbilityInfo.TriggerActionType == ECharacterActionType::E_NONE)
    {
        return false;
    }
    return true;
}

void AAbilityExtraActionBase::BindtoActionTrackingEvents(FAbilityInfoStruct& NewApplicationInfo)
{
    if (NewApplicationInfo.AbilityName == NAME_None || !ActionTrackingComponentRef.IsValid())
    {
        UE_LOG(LogAbility, Error, TEXT("AAbilityExtraActionBase::BindtoActionTrackingEvents / Invalid AbilityName or ActionTrackingComponentRef"));
        return;
    }

    if (!NewApplicationInfo.bIsRequireExtraAction) return;
    FActionStateUpdateDelegate& actionStateUpdateDelegate = ActionTrackingComponentRef.Get()->GetActionStateUpdateDelegate(NewApplicationInfo.TriggerActionType);
    actionStateUpdateDelegate.AddDynamic(this, &AAbilityExtraActionBase::OnTriggerActionStateUpdated);
	UE_LOG(LogAbility, Log, TEXT("AAbilityExtraActionBase::BindtoActionTrackingEvents - Bound to ActionStateUpdateDelegate for ActionName: %s"), *NewApplicationInfo.GetActionTriggerName());
}

void AAbilityExtraActionBase::UnbindFromActionTrackingEvents(FAbilityInfoStruct& OldApplicationInfo)
{
    if (OldApplicationInfo.AbilityName == NAME_None || !ActionTrackingComponentRef.IsValid())
    {
		UE_LOG(LogAbility, Error, TEXT("UAbilityExtraActionBase::UnbindFromActionTrackingEvents / Invalid AbilityName or ActionTrackingComponentRef"));
        return;
    }

	if (!OldApplicationInfo.bIsRequireExtraAction) return;
    FActionStateUpdateDelegate& actionStateUpdateDelegate = ActionTrackingComponentRef.Get()->GetActionStateUpdateDelegate(OldApplicationInfo.TriggerActionType);
	actionStateUpdateDelegate.RemoveDynamic(this, &AAbilityExtraActionBase::OnTriggerActionStateUpdated);
}
