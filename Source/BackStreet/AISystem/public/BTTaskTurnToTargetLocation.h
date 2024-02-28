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
	//BT�� �����尡 ������ �Ǿ����� ���� ��츦 ����
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UFUNCTION()
		FRotator GetTurnRotation(APawn* ControlledPawn);

	UFUNCTION()
		void LogMessage(FString str, FVector2D vec);

private:
	//������ ĳ���� ������
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//������ ������Ʈ
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;
};
