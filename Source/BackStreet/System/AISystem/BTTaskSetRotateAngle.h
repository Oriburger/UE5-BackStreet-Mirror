// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
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
	//BT에 블랙보드가 설정이 되어있지 않은 경우를 방지
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
	//소유자 캐릭터 약참조
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//블랙보드 컴포넌트
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;

	//Flag that means its turn is right or left.	
	bool bIsRightTurn;
};
