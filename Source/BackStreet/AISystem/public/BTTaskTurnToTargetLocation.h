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

	UPROPERTY(EditInstanceOnly)
		FBlackboardKeySelector TargetCharacterBBKey;

	UPROPERTY(EditInstanceOnly)
		FBlackboardKeySelector TargetLocationBBKey;

protected:
	//BT에 블랙보드가 설정이 되어있지 않은 경우를 방지
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	//TargetCharacter이 Valid라면 TargetCharacter의 WorldLocation을 Target로 지정
	UFUNCTION()
		void InitTargetLocationFromBBKey();
	
	//이번 tick에 돌 Angle을 반환
	UFUNCTION()
		double CalculateTurnAngle(const FVector& OwnerLocation, const FVector& OwnerForwardVector);

	UFUNCTION()
		void LogMessage(FString str, FVector2D vec);

private:
	//소유자 캐릭터 약참조
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//블랙보드 컴포넌트
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;

	UPROPERTY()
		FVector TargetLocation;
};
