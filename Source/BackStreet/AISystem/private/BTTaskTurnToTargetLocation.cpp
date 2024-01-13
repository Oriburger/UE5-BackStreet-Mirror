// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/BTTaskTurnToTargetLocation.h"
#include "../../Character/public/EnemyCharacterBase.h"
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
        TargetCharacterBBKey.ResolveSelectedKey(*bbAsset);
        TargetLocationBBKey.ResolveSelectedKey(*bbAsset);
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
    BlackboardRef = OwnerComp.GetBlackboardComponent();
    
    InitTargetLocationFromBBKey();
    
    OwnerCharacterRef->SetActorRotation(GetTurnRotation(OwnerCharacterRef.Get()));
 
    return EBTNodeResult::Succeeded;
}

void UBTTaskTurnToTargetLocation::InitTargetLocationFromBBKey()
{
    FName bbKeyName = TargetCharacterBBKey.SelectedKeyName;
    const AActor* targetCharacterRef = Cast<AActor>(BlackboardRef.Get()->GetValueAsObject(bbKeyName));

    if (IsValid(targetCharacterRef) && !targetCharacterRef->IsActorBeingDestroyed())
    {
        TargetLocation = targetCharacterRef->GetActorLocation();
    }
    else
    {
        bbKeyName = TargetLocationBBKey.SelectedKeyName;
        TargetLocation = BlackboardRef.Get()->GetValueAsVector(bbKeyName);
    }
}

FRotator UBTTaskTurnToTargetLocation::GetTurnRotation(APawn* ControlledPawn)
{
    if(!IsValid(ControlledPawn)) return FRotator();

    FVector directionToTarget = TargetLocation - ControlledPawn->GetActorLocation();
    directionToTarget.Z = 0.0f;

    FRotator pawnRotation = ControlledPawn->GetActorRotation(); 
    FRotator targetRotation = UKismetMathLibrary::MakeRotFromX(directionToTarget);
    
    return UKismetMathLibrary::RInterpTo(pawnRotation, targetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
}

void UBTTaskTurnToTargetLocation::LogMessage(FString str, FVector2D vec)
{
    UE_LOG(LogTemp, Warning, TEXT("%s) - {%.2lf, %.2lf}"), *str, vec.X, vec.Y);
}
