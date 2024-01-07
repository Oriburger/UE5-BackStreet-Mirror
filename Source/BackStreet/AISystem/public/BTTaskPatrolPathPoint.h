// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTaskPatrolPathPoint.generated.h"

/**
  - Name        : UBTTaskPatrolPathPoint
  - Descirption : 
  - Date        : 2024/01/07 LJH
 */
UCLASS()
class BACKSTREET_API UBTTaskPatrolPathPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskPatrolPathPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
		void LogMessage(FString str, FVector2D vec);

	//Move to next patrol point include check after chase event
	UFUNCTION()
		void MoveToPatrolPoint();

	//Post Event of above function "MoveToPatrolPoint"
	UFUNCTION()
		void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

protected:
	//소유자 캐릭터 약참조
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//블랙보드 컴포넌트
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;

private:
	UPROPERTY()
		class UAIPathPatrolComponent* PathPatrolComponent;

	//TimerHandle for wait in each patrol path point
	UPROPERTY()
		FTimerHandle PointWaitTimerHandle;

	UPROPERTY()
		bool bIsFirstEvent = true;
	
	UPROPERTY()
		bool bIsChaseEnd;
};
