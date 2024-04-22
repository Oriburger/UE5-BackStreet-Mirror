// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/BTSTurnToTarget.h"
#include "../../Character/public/EnemyCharacterBase.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Kismet/KismetMathLibrary.h"
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
		OwnerCharacterRef->SetActorRotation(GetTurnRotation(OwnerCharacterRef.Get()));
	}
}

FRotator UBTSTurnToTarget::GetTurnRotation(APawn* ControlledPawn)
{
	if (!IsValid(ControlledPawn)) return FRotator();

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
			return FRotator();
		targetLocation = targetActor->GetActorLocation();
	}
	else if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		targetLocation = BlackboardRef->GetValueAsVector(TargetBBKey.SelectedKeyName);
	else
		return FRotator();

	//Get target rotation using target location.
	FVector directionToTarget = targetLocation - ControlledPawn->GetActorLocation();
	directionToTarget.Z = 0.0f;

	FRotator pawnRotation = ControlledPawn->GetActorRotation();
	FRotator targetRotation = UKismetMathLibrary::MakeRotFromX(directionToTarget);

	return UKismetMathLibrary::RInterpTo(pawnRotation, targetRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
}