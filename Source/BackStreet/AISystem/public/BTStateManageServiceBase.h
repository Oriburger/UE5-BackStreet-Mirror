// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "BehaviorTree/BTService.h"
#include "BTStateManageServiceBase.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UBTStateManageServiceBase : public UBTService
{
	GENERATED_BODY()
public:
	UBTStateManageServiceBase(const FObjectInitializer& ObjectInitializer);

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override; 

protected:
	UFUNCTION()
		void UpdateAIState();

	UFUNCTION()
		bool CheckPatrolState();

	UFUNCTION()
		bool CheckReturnState();

	UFUNCTION()
		bool CheckChaseState();

	UFUNCTION()
		bool CheckAttackState();

	UFUNCTION()
		float GetDistanceTo(const FVector& EndLocation);

	UFUNCTION()
		void OnOwnerGetDamaged(AActor* Causer);

private:
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;

	UPROPERTY()
		EAIBehaviorType AIBehaviorState;
};
