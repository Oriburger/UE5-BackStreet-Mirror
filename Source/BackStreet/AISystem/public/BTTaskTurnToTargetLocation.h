// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTaskTurnToTargetLocation.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UBTTaskTurnToTargetLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskTurnToTargetLocation();

	//Target to rotate. This value can be vector, actor, rotator.
	UPROPERTY(EditInstanceOnly)
		FBlackboardKeySelector TargetBBKey;

protected:
	//BT에 블랙보드가 설정이 되어있지 않은 경우를 방지
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UFUNCTION()
		FRotator GetTurnRotation(APawn* ControlledPawn);

	UFUNCTION()
		void LogMessage(FString str, FVector2D vec);

private:
	//소유자 캐릭터 약참조
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//블랙보드 컴포넌트
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;
};
