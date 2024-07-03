// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTSTurnToTarget.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UBTSTurnToTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTSTurnToTarget();

	//Target to rotate. This value can be vector, actor, rotator.
	UPROPERTY(EditInstanceOnly)
		FBlackboardKeySelector TargetBBKey;

public:

	// Left OR Right 회전방향 Return
	// True -> Right, False -> Left
	UFUNCTION(BlueprintCallable)
	bool GetTurnDirection(bool GetRightLeft);

protected:
	//BT에 블랙보드가 설정이 되어있지 않은 경우를 방지
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:

	UFUNCTION()
	FRotator GetTurnRotation(APawn* ControlledPawn);

private:

	//오른쪽 또는 왼쪽 회전방향 Check 후 Set
	UFUNCTION()
	void CheckTurnDirection(bool Right, bool Left);

private:
	//소유자 캐릭터 약참조
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//블랙보드 컴포넌트
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;

	//캐릭터 AIContorllerBase
	class AAIControllerBase* CharAIController;

	//Turn Rigt? OR Left?
	bool TurnRight;

	bool TurnLeft;

	float Direction;

	FVector ForwardVector;
};
