// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/BTTaskTurnToTargetLocation.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIController.h"

#define RANGED_WEAPON_ERROR_TOLERANCE 0.1f
#define MELEE_WEAPON_ERROR_TOLERANCE 0.5f

typedef UKismetMathLibrary UKML;


UBTTaskTurnToTargetLocation::UBTTaskTurnToTargetLocation()
{
    NodeName = "BTT_TurnToTarget";
}

void UBTTaskTurnToTargetLocation::InitializeFromAsset(UBehaviorTree& Asset)
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
        check(0);
    }
}   
    
EBTNodeResult::Type UBTTaskTurnToTargetLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{   
    Super::ExecuteTask(OwnerComp, NodeMemory);
    OwnerCharacterRef = Cast<AEnemyCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
    if(!OwnerCharacterRef.IsValid()) return EBTNodeResult::Failed;

    BlackboardRef = OwnerComp.GetBlackboardComponent();
    OwnerCharacterRef->SetActorRotation(GetTurnRotation(OwnerCharacterRef.Get()));

    return EBTNodeResult::Succeeded;
}   
    
FRotator UBTTaskTurnToTargetLocation::GetTurnRotation(APawn* ControlledPawn)
{   
    if (!IsValid(ControlledPawn)) return FRotator();
    
    FVector targetLocation;
    if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Rotator::StaticClass())
        return BlackboardRef->GetValueAsRotator(TargetBBKey.SelectedKeyName);
    else if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
        targetLocation = Cast<AActor>(BlackboardRef->GetValueAsObject(TargetBBKey.SelectedKeyName))->GetActorLocation();
    else if (TargetBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
        targetLocation = BlackboardRef->GetValueAsVector(TargetBBKey.SelectedKeyName);
    else 
        return FRotator();

    FVector directionToTarget = targetLocation - ControlledPawn->GetActorLocation();
    directionToTarget.Z = 0.0f;

    FRotator pawnRotation = ControlledPawn->GetActorRotation(); 
    FRotator targetRotation = UKismetMathLibrary::MakeRotFromX(directionToTarget);
    
    return UKismetMathLibrary::RInterpTo(pawnRotation, targetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
}   
    
void UBTTaskTurnToTargetLocation::LogMessage(FString str, FVector2D vec)
{
    UE_LOG(LogTemp, Warning, TEXT("%s) - {%.2lf, %.2lf}"), *str, vec.X, vec.Y);
}
