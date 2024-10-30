// Fill out your copyright notice in the Description page of Project Settings.


#include "BTSTurnToTarget.h"
#include "../../Character/EnemyCharacter/EnemyCharacterBase.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "AIControllerBase.h"
#include "AIController.h"

typedef UKismetMathLibrary UKML;


UBTSTurnToTarget::UBTSTurnToTarget()
{
	NodeName = "BTS_TurnToTarget";
}

void UBTSTurnToTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* bbAsset = GetBlackboardAsset();
	if (bbAsset != nullptr)
	{
		TargetBBKey.ResolveSelectedKey(*bbAsset);
	}
	else
	{
		UE_LOG(LogBehaviorTree, Warning, TEXT("BTTask_%s 초기화 실패."), *GetName());
	}
}

void UBTSTurnToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	OwnerCharacterRef = Cast<AEnemyCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!OwnerCharacterRef.IsValid()) return;

	BlackboardRef = OwnerComp.GetBlackboardComponent();
	if (OwnerCharacterRef.Get()->GetIsActionActive(ECharacterActionType::E_Idle)
		|| OwnerCharacterRef.Get()->GetIsActionActive(ECharacterActionType::E_Skill))
	{
		//OwnerCharacterRef->SetActorRotation(GetTurnRotation(OwnerCharacterRef.Get()));
	}

	//Turn In Place를 위한 회전방향 구하기
	if (OwnerCharacterRef->GetVelocity().Length() == 0){
		float dot = FVector::DotProduct(OwnerCharacterRef->GetActorRightVector(), ForwardVector);
		Direction = dot;
		ForwardVector = OwnerCharacterRef->GetActorForwardVector();

		if (Direction > 0.007) {
			CheckTurnDirection(false, true);
		}
		else TurnLeft = false;

		if (Direction < -0.007) {
			CheckTurnDirection(true, false);
		}
		else TurnRight = false;
	}
	else {
		CheckTurnDirection(false, false);
	}
	CharAIController = Cast<AAIControllerBase>(OwnerCharacterRef->GetController());
	CharAIController->SetTurnDirection(TurnRight, TurnLeft);

}

//Get target location for calcurate rotation value to target
FRotator UBTSTurnToTarget::GetTurnRotation(APawn* ControlledPawn)
{
	if (!IsValid(ControlledPawn)) return FRotator::ZeroRotator;

	if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Rotator::StaticClass())
	{
		FRotator pawnRotation = ControlledPawn->GetActorRotation();
		FRotator targetRotation = BlackboardRef->GetValueAsRotator(TargetBBKey.SelectedKeyName);
		if (UKismetMathLibrary::EqualEqual_RotatorRotator(pawnRotation, targetRotation, 2.0f))
			return targetRotation;
		return UKismetMathLibrary::RInterpTo(pawnRotation, targetRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
	}

	FVector targetLocation;
	if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		AActor* targetActor = Cast<AActor>(BlackboardRef->GetValueAsObject(TargetBBKey.SelectedKeyName));
		if (!IsValid(targetActor))
			return FRotator::ZeroRotator;
		targetLocation = targetActor->GetActorLocation();
	}
	else if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		targetLocation = BlackboardRef->GetValueAsVector(TargetBBKey.SelectedKeyName);
	else
		return FRotator::ZeroRotator;

	//Get target rotation using target location.
	FVector directionToTarget = targetLocation - ControlledPawn->GetActorLocation();
	directionToTarget.Z = 0.0f;

	FRotator pawnRotation = ControlledPawn->GetActorRotation();
	FRotator targetRotation = UKismetMathLibrary::MakeRotFromX(directionToTarget);

	return UKismetMathLibrary::RInterpTo(pawnRotation, targetRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
}

bool UBTSTurnToTarget::GetTurnDirection(bool GetRightLeft) 
{
	if (GetRightLeft) return TurnRight;
	else return TurnLeft;
}

void UBTSTurnToTarget::CheckTurnDirection(bool Right, bool Left) 
{
	TurnRight = Right;
	TurnLeft = Left;
}