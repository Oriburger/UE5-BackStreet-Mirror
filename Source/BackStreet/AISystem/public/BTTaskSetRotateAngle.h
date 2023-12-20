// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTaskSetRotateAngle.generated.h"

/**
  - Name        : UBTTaskSetRotateAngle
  - Descirption : Specify rotate angle to turn.
                  After execute this node, you have to execute BTT_TurnToTarget.
  - Date        : 2023/12/19 LJH
 */
UCLASS()
class BACKSTREET_API UBTTaskSetRotateAngle : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskSetRotateAngle();

	UPROPERTY(EditInstanceOnly)
		FBlackboardKeySelector ChaseEndFlagBBKey;
	
	UPROPERTY(EditInstanceOnly)
		FBlackboardKeySelector TargetRotationBBKey;

protected:
	//BT�� �����尡 ������ �Ǿ����� ���� ��츦 ����
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UFUNCTION()
		void LogMessage(FString str, FVector2D vec);

	UFUNCTION()
		bool GetIsPatrolTaskDone();

protected:
	//L, R
	UPROPERTY(EditInstanceOnly)
		float RotateAngleLeft;
	
	UPROPERTY(EditInstanceOnly)
		float RotateAngleRight;

private:
	//������ ĳ���� ������
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//������ ������Ʈ
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;

	//Flag that means its turn is right or left.	
	bool bIsRightTurn;
};
