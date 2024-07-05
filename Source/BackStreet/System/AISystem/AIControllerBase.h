// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "AIController.h"
#include "AIControllerBase.generated.h"


/**
 * 
 */
UCLASS()
class BACKSTREET_API AAIControllerBase : public AAIController
{
	GENERATED_BODY()

//-- �⺻ �Լ� �� ������Ʈ ---------------
public:
	AAIControllerBase();

	UFUNCTION()
		virtual void OnPossess(APawn* PossessedPawn) override;

	UFUNCTION()
		virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		void ActivateAI();

	UFUNCTION(BlueprintCallable)
		void DeactivateAI();

	UFUNCTION(BlueprintCallable)
		bool GetTurnDirection(bool Direction);
		
	UFUNCTION()
		void SetTurnDirection(bool Right, bool Left);

private:

	bool TurnRight;

	bool TurnLeft;

//--Perception ���� ---------------------
public:
	UFUNCTION(BlueprintCallable)
		void UpdateTargetPerception(AActor* Actor, FAIStimulus Stimulus);

protected:
	UPROPERTY(EditDefaultsOnly)
		float MaxSightAge = 5.0f;

private:
	UFUNCTION()
		void ProcessSight(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
		void ProcessHearing(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
		void ProcessPrediction(AActor* Actor, FAIStimulus Stimulus);

private:
	UFUNCTION()
		void ResetTargetPerception();

//-- Blackboard ���� ----------------------
public:
	UFUNCTION()
		void UpdateNewWeapon();

//--�ڿ� ���� ---------------------
public:
	//���� ���� �̺�Ʈ�� ���ε��ؼ� ���
	UFUNCTION()
		void ClearAllTimerHandle();

//--��Ÿ Property-----------------------
protected:
	//������ BT�� ����
	UPROPERTY(EditDefaultsOnly, Category = "AI")
		UBehaviorTree* BehaviorTree;

	//AI�� ���� ������ �ִ� BT �׼� ���¸� ��Ÿ��
	UPROPERTY(BlueprintReadWrite)
		EAIBehaviorType AIBehaviorState;

private:
	//ĳ���� ���� Time-Out ��, ����� ����� ������ ����
	//�ش� �Լ��� ���ε��Ͽ� ����� Ÿ�̸� �ڵ�
	UPROPERTY()
		FTimerHandle SightLossTimerHandle;
};
