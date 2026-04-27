// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "AbilityExtraActionBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateOnExtraActionStateUpdated, int32, AbilityID, AAbilityExtraActionBase*, ExtraActionInstance);

UCLASS()
class BACKSTREET_API AAbilityExtraActionBase : public AActor
{
	GENERATED_BODY()

//====================================================================
//====== Common & Delegate ===========================================
public:
	AAbilityExtraActionBase();
	
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateOnExtraActionStateUpdated OnExtraActionTriggered;
	
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateOnExtraActionStateUpdated OnExtraActionCompleted;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateOnExtraActionStateUpdated OnExtraActionLifeSpanEnded;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
		void OnTriggerActionStateUpdated(ECharacterActionType ActionType, uint8 PrevActionState, uint8 ActionState);

	UFUNCTION()
		void OnAbilityInfoUpdated(int32 AbilityID, FAbilityInfoStruct NewAbilityInfo);

//======================================================================
//====== Basic Function ================================================
public:
	UFUNCTION()
		void InitializeExtraAction(class AMainCharacterBase* NewOwnerCharacter, class UAbilityManagerComponent* NewAbilityManagerComponent, FAbilityInfoStruct NewLinkedAbilityInfo);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void ExecuteExtraAction();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void StopExtraAction();

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetTriggerValidity();

	UFUNCTION()
		void BindtoActionTrackingEvents(FAbilityInfoStruct& NewApplicationInfo);

	UFUNCTION()
		void UnbindFromActionTrackingEvents(FAbilityInfoStruct& OldApplicationInfo);

//======================================================================
//====== Property ======================================================
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
		bool bPrintDebugLog = false;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
		//bool bIsAutoDestroyAfterComplete = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
		bool bIsExecuting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
		FAbilityInfoStruct LinkedAbilityInfo;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		AMainCharacterBase* GetOwnerCharacter() const { return OwnerCharacterRef.IsValid() ? OwnerCharacterRef.Get() : nullptr; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool IsExecuting() const { return bIsExecuting; }

protected:
	TWeakObjectPtr<class AMainCharacterBase> OwnerCharacterRef;
	
	TWeakObjectPtr<class UActionTrackingComponent> ActionTrackingComponentRef;
	
	TWeakObjectPtr<class UAbilityManagerComponent> AbilityManagerComponentRef;

private:
	FTimerHandle DefferTimerHandle;
	FTimerHandle AutoExecuteTimerHandle;
};