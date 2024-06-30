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

	// Left OR Right ȸ������ Return
	// True -> Right, False -> Left
	UFUNCTION(BlueprintCallable)
	bool GetTurnDirection(bool GetRightLeft);

protected:
	//BT�� �����尡 ������ �Ǿ����� ���� ��츦 ����
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:

	UFUNCTION()
	FRotator GetTurnRotation(APawn* ControlledPawn);

private:

	//������ �Ǵ� ���� ȸ������ Check �� Set
	UFUNCTION()
	void CheckTurnDirection(bool Right, bool Left);

private:
	//������ ĳ���� ������
	TWeakObjectPtr<class AEnemyCharacterBase> OwnerCharacterRef;

	//������ ������Ʈ
	TWeakObjectPtr<UBlackboardComponent> BlackboardRef;

	//ĳ���� AIContorllerBase
	class AAIControllerBase* CharAIController;

	//Turn Rigt? OR Left?
	bool TurnRight;

	bool TurnLeft;

	float Direction;

	FVector ForwardVector;
};
