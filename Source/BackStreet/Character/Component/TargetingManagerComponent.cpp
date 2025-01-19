// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetingManagerComponent.h"
#include "../MainCharacter/MainCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../CharacterBase.h"
#include "ActionTrackingComponent.h"

// Sets default values for this component's properties
UTargetingManagerComponent::UTargetingManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UTargetingManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// init parent
	OwnerCharacter = Cast<ACharacterBase>(GetOwner());

	// if parent is player, set following camera ref
	if (OwnerCharacter.IsValid() && OwnerCharacter->ActorHasTag("Player"))
	{
		FollowingCameraRef = Cast<AMainCharacterBase>(OwnerCharacter.Get())->FollowingCamera;
	}

	if (bAutoTargeting)
	{
		GetOwner()->GetWorldTimerManager().SetTimer(AutoFindTimerHandle, this, &UTargetingManagerComponent::UpdateTargetedCandidate, AutoTargetingRate, true);
	}
}

AActor* UTargetingManagerComponent::FindNearEnemyToTarget(float RadiusOverride)
{
	if (!OwnerCharacter.IsValid() || !FollowingCameraRef.IsValid()) return nullptr;

	FVector traceDirection = FollowingCameraRef.Get()->GetForwardVector(); traceDirection.Z = 0.0f;
	FVector startLocation = OwnerCharacter.Get()->GetActorLocation() + traceDirection * 150.0f;
	FVector endLocation = startLocation + traceDirection * MaxFindDistance;
	TArray<FHitResult> hitResultList;
	TArray<AActor*> actorToIgnore = { OwnerCharacter.Get() };
	if (bIsTargetingActivated && TargetedCharacter.IsValid())
	{
		actorToIgnore.Add(TargetedCharacter.Get());
	}

	//TargetedCandidate가 있다면 && 공격 중일때 (막 끝났을때)
	if (TargetedCandidate.IsValid() && !OwnerCharacter.Get()->ActionTrackingComponent->GetIsActionReady("Attack"))
	{
		//
		traceDirection = TargetedCandidate.Get()->GetActorForwardVector() * -1.0f;
		startLocation = OwnerCharacter.Get()->HitSceneComponent->GetComponentLocation() + traceDirection * 50.0f;;
		endLocation = startLocation + traceDirection * MaxFindDistance;
	}
	

	//trace 진행
	UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(), startLocation, endLocation, RadiusOverride <= 1.0f ? TraceRadius : RadiusOverride, "Pawn", true
		, actorToIgnore, bShowDebugSphere ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitResultList, true, FColor::Green, FColor::Red, AutoTargetingRate);

	float minDist = FLT_MAX;
	ACharacterBase* target = nullptr;
	
	bool bContainsOldCandidate = false; //타게팅이 잡힌애가 또 트레이스에 잡힌다면?
	for (FHitResult& result : hitResultList)
	{
		if (!result.bBlockingHit) continue;

		AActor* pawn = result.GetActor();
		if (pawn == TargetedCandidate && pawn != nullptr)
		{
			bContainsOldCandidate = true;
		}

		if (!IsValid(pawn)) continue;
		if (!pawn->Tags.IsValidIndex(1) || !OwnerCharacter.Get()->Tags.IsValidIndex(1)) continue;
		if (pawn->Tags[1] == OwnerCharacter.Get()->Tags[1]) continue;

		float dist = FVector::Distance(pawn->GetActorLocation(), startLocation);
		if (dist < minDist)
		{
			dist = minDist;
			ACharacterBase* temp = Cast<ACharacterBase>(pawn);
			if (temp->GetIsActionActive(ECharacterActionType::E_Die)) continue;
			target = Cast<ACharacterBase>(pawn);
		}
	}

	//포함이 되어있다면 그냥 그 폰을 반환
	if (bContainsOldCandidate)
	{
		return TargetedCandidate.Get();
	}
	return target;
}

void UTargetingManagerComponent::ForceTargetingToNearestCharacter()
{
	FindNearEnemyToTarget(500.0f);
	ActivateTargeting();
}

void UTargetingManagerComponent::ActivateTargeting()
{
	if (!TargetedCandidate.IsValid()) return;
	if (bIsTargetingActivated && TargetedCharacter == TargetedCandidate)
	{
		DeactivateTargeting();
		return;
	}
	TargetedCharacter = TargetedCandidate;

	bIsTargetingActivated = true;
	//GetOwner()->GetWorldTimerManager().ClearTimer(CameraRotateTimerHandle);
	//GetOwner()->GetWorldTimerManager().SetTimer(CameraRotateTimerHandle, this, &UTargetingManagerComponent::UpdateCameraRotation, 0.01f, true);

	TargetedCharacter.Get()->GetCharacterMovement()->bOrientRotationToMovement = false;
	TargetedCharacter.Get()->GetCharacterMovement()->bUseControllerDesiredRotation = true;

	OnTargetingActivated.Broadcast(true, TargetedCharacter.Get());
}

void UTargetingManagerComponent::DeactivateTargeting()
{
	GetOwner()->GetWorldTimerManager().ClearTimer(CameraRotateTimerHandle);
	CameraRotateTimerHandle.Invalidate();

	OwnerCharacter.Get()->GetCharacterMovement()->bOrientRotationToMovement = true;
	OwnerCharacter.Get()->GetCharacterMovement()->bUseControllerDesiredRotation = false;

	bIsTargetingActivated = false;
	if (FollowingCameraRef.IsValid())
	{
		//FollowingCameraRef.Get()->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator, false);
	}
	OnTargetingActivated.Broadcast(false, nullptr);
}

void UTargetingManagerComponent::UpdateTargetedCandidate()
{
	AActor* newCandidate = FindNearEnemyToTarget();
	
	//후보 업데이트는 다음과 같은 조건일때 수행한다 
	//기본적으로 netCandidate가 valid할 때 (정상적으로 찾았을때)
	//현재 공격중이 아닐때 혹은 아무도 없을때
	if (!TargetedCandidate.IsValid() || OwnerCharacter.Get()->ActionTrackingComponent->GetIsActionReady("Attack"))
	{
		TargetedCandidate = Cast<ACharacterBase>(newCandidate);
		OnTargetUpdated.Broadcast(TargetedCandidate.Get());
	}

	if (TargetedCandidate.IsValid() && !OwnerCharacter.Get()->GetCharacterGameplayInfo().bIsAirAttacking)
	{
		if (FVector::Dist(OwnerCharacter.Get()->GetActorLocation(), TargetedCandidate.Get()->GetActorLocation()) > TargetingMaintainThreashold)
		{
			TargetedCandidate.Reset();
			DeactivateTargeting();
			return;
		}
	}
}

void UTargetingManagerComponent::UpdateCameraRotation()
{
	if (!FollowingCameraRef.IsValid() || !TargetedCharacter.IsValid() || !OwnerCharacter.IsValid()) return;
	if (OwnerCharacter.Get()->GetIsActionActive(ECharacterActionType::E_Die) || TargetedCharacter.Get()->GetIsActionActive(ECharacterActionType::E_Die))
	{
		DeactivateTargeting();
		return;
	}

	if (FVector::Dist(OwnerCharacter.Get()->GetActorLocation(), TargetedCharacter.Get()->GetActorLocation()) >= 50.0f)
	{
		FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(FollowingCameraRef.Get()->GetComponentLocation(), TargetedCharacter.Get()->GetActorLocation());
		newRotation.Roll = 0.0f;
		OwnerCharacter.Get()->GetController()->SetControlRotation(newRotation);
	}
}

ACharacterBase* UTargetingManagerComponent::GetTargetedCharacter()
{
	if (TargetedCharacter.IsValid())
	{
		return TargetedCharacter.Get();
	}
	return nullptr;
}

void UTargetingManagerComponent::SetTargetedCharacter(ACharacterBase* NewTarget)
{
	TargetedCharacter = NewTarget;
}

