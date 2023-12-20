// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTaskSetRotateAngle.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UBTTaskSetRotateAngle : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskSetRotateAngle();

	UPROPERTY(EditInstanceOnly)
		FBlackboardKeySelector TargetCharacterBBKey;

	UPROPERTY(EditInstanceOnly)
		FBlackboardKeySelector TargetLocationBBKey;

protected:
	//BT�� �����尡 ������ �Ǿ����� ���� ��츦 ����
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UFUNCTION()
		void LogMessage(FString str, FVector2D vec);

private:
	//������ ĳ���� ������
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//������ ������Ʈ
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;

	UPROPERTY()
		FVector TargetLocation;
};
