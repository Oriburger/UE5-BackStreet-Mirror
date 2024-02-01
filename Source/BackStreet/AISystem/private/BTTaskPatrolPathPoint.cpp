// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/BTTaskPatrolPathPoint.h"
#include "../public/AIPathPatrolComponent.h"
#include "../public/PathPatrolPoint.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIController.h"

typedef UKismetMathLibrary UKML;

UBTTaskPatrolPathPoint::UBTTaskPatrolPathPoint()
{
    NodeName = "BTT_PatrolPathPoint";
}
    
EBTNodeResult::Type UBTTaskPatrolPathPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{   
    Super::ExecuteTask(OwnerComp, NodeMemory);

    //Check is the first execution of task and chase task is ended;
    //bIsChaseEnd = !bIsFirstEvent;
    bIsFirstEvent = false;

    //Try to get character reference
    OwnerCharacterRef = Cast<AEnemyCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
    if(!OwnerCharacterRef.IsValid()) return EBTNodeResult::Failed;

    //Try to get pathPatrolComponent from owner character
    PathPatrolComponent = OwnerCharacterRef->GetComponentByClass<UAIPathPatrolComponent>();
    if (!IsValid(PathPatrolComponent)) return EBTNodeResult::Failed;
    
    //Check path patrol component is valid
    if(PathPatrolComponent->PatrolPathInfoList.IsEmpty()) return EBTNodeResult::Failed;
    
    //Try move to patrol point
    UE_LOG(LogTemp, Warning, TEXT("UBTTaskPatrolPathPoint::ExecuteTask() : Start move"));
    MoveToPatrolPoint();

    BlackboardRef = OwnerComp.GetBlackboardComponent();
    UE_LOG(LogTemp, Warning, TEXT("UBTTaskPatrolPathPoint::MoveToPatrolPoint() : real Fin move"));
    return EBTNodeResult::Succeeded;
}   

void UBTTaskPatrolPathPoint::LogMessage(FString str, FVector2D vec)
{
    UE_LOG(LogTemp, Warning, TEXT("%s) - {%.2lf, %.2lf}"), *str, vec.X, vec.Y);
}

void UBTTaskPatrolPathPoint::MoveToPatrolPoint()
{
    if (!IsValid(PathPatrolComponent)) return;

    //Get nearest patrol point from pathPatrolComponent
    //And check if it is valid
    TPair<APathPatrolPoint*, int32> patrolPointInfo;
    
    if (bIsChaseEnd)
    {
        bIsChaseEnd = false;
        patrolPointInfo = PathPatrolComponent->GetNearestPointInfo();
        PathPatrolComponent->CurrentPointIdx = patrolPointInfo.Value;
    }
    else
    {
        patrolPointInfo.Key = PathPatrolComponent->PatrolPathInfoList[PathPatrolComponent->CurrentPointIdx].Point;
    }

    if (!IsValid(patrolPointInfo.Key))
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTaskPatrolPathPoint::MoveToPatrolPoint() : failed to get nstp"));
        return;
    }

    //Try get ai controller ref, and check validity 
    AAIController* aiControllerRef = UAIBlueprintHelperLibrary::GetAIController(OwnerCharacterRef.Get());
    if (!IsValid(aiControllerRef))
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTaskPatrolPathPoint::MoveToPatrolPoint() : failed to get ai controller"));
        return;
    }

    aiControllerRef->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTaskPatrolPathPoint::OnMoveCompleted);
    aiControllerRef->ReceiveMoveCompleted.AddDynamic(this, &UBTTaskPatrolPathPoint::OnMoveCompleted);

    UAIBlueprintHelperLibrary::CreateMoveToProxyObject(GetWorld(), OwnerCharacterRef.Get()
                                                       , patrolPointInfo.Key->GetActorLocation(), nullptr, 5.0f);
}


void UBTTaskPatrolPathPoint::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    if (Result != EPathFollowingResult::Success) return;
    if (!IsValid(PathPatrolComponent)) return;

    int32 idx = PathPatrolComponent->CurrentPointIdx %= PathPatrolComponent->PatrolPathInfoList.Num();
    float delayTime = PathPatrolComponent->PatrolPathInfoList[idx].DelayValue;

    if (GetWorld()->GetTimerManager().IsTimerActive(PointWaitTimerHandle)) return;
    
    GetWorld()->GetTimerManager().SetTimer(PointWaitTimerHandle, FTimerDelegate::CreateLambda([&]() {
        //Update Index
        PathPatrolComponent->CurrentPointIdx += 1;
        PathPatrolComponent->CurrentPointIdx %= PathPatrolComponent->PatrolPathInfoList.Num();

        UE_LOG(LogTemp, Warning, TEXT("UBTTaskPatrolPathPoint::OnMoveCompleted %d"), PathPatrolComponent->CurrentPointIdx);
        MoveToPatrolPoint();

        // TimerHandle Init
        GetWorld()->GetTimerManager().ClearTimer(PointWaitTimerHandle);
     }), delayTime, false); 
}