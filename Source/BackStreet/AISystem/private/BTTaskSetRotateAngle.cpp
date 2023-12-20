// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/BTTaskSetRotateAngle.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIController.h"

#define RANGED_WEAPON_ERROR_TOLERANCE 0.1f
#define MELEE_WEAPON_ERROR_TOLERANCE 0.5f

typedef UKismetMathLibrary UKML;


UBTTaskSetRotateAngle::UBTTaskSetRotateAngle()
{
    NodeName = "BTT_SetRotateAngle";
}

void UBTTaskSetRotateAngle::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    UBlackboardData* bbAsset = GetBlackboardAsset();
    if (bbAsset != nullptr)
    {
        TargetRotationBBKey.ResolveSelectedKey(*bbAsset);
        ChaseEndFlagBBKey.ResolveSelectedKey(*bbAsset);
    }
    else
    {
        UE_LOG(LogBehaviorTree, Warning, TEXT("BTTask_%s 초기화 실패."), *GetName());
        check(0);
    }
}

EBTNodeResult::Type UBTTaskSetRotateAngle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (Super::ExecuteTask(OwnerComp, NodeMemory)) return EBTNodeResult::Failed;

    //Initialize essential member
    OwnerCharacterRef = Cast<AEnemyCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
    BlackboardRef = OwnerComp.GetBlackboardComponent();
    if (!OwnerCharacterRef.IsValid() || !BlackboardRef.IsValid())
        return EBTNodeResult::Failed;

    //Check return task is done.
    if (!GetIsPatrolTaskDone())
        return EBTNodeResult::Failed;
       
    float targetAngle = OwnerCharacterRef.Get()->GetActorRotation().Yaw;
    targetAngle = bIsRightTurn ? targetAngle + RotateAngleRight : targetAngle - RotateAngleLeft;
    
    BlackboardRef->SetValueAsRotator(TargetRotationBBKey.SelectedKeyName, FRotator(0, targetAngle, 0));
    bIsRightTurn = !bIsRightTurn;

    return EBTNodeResult::Succeeded;
}

void UBTTaskSetRotateAngle::LogMessage(FString str, FVector2D vec)
{
    UE_LOG(LogTemp, Warning, TEXT("%s) - {%.2lf, %.2lf}"), *str, vec.X, vec.Y);
}

bool UBTTaskSetRotateAngle::GetIsPatrolTaskDone()
{
    return BlackboardRef->GetValueAsBool(ChaseEndFlagBBKey.SelectedKeyName);
}