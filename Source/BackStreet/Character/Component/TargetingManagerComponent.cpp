// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetingManagerComponent.h"
#include "../MainCharacter/MainCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../CharacterBase.h"

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
		GetOwner()->GetWorldTimerManager().SetTimer(AutoFindTimerHandle, this, &UTargetingManagerComponent::UpdateTargetedCandidate, 0.5f, true);
	}
}

AActor* UTargetingManagerComponent::FindNearEnemyToTarget(float Radius)
{
	if (!OwnerCharacter.IsValid() || !FollowingCameraRef.IsValid()) return nullptr;

	FVector startLocation = OwnerCharacter.Get()->GetActorLocation();
	FVector endLocation = startLocation + FollowingCameraRef.Get()->GetForwardVector() * MaxFindDistance;
	TArray<FHitResult> hitResultList;
	TArray<AActor*> actorToIgnore = { OwnerCharacter.Get() };
	if (bIsTargetingActivated && TargetedCharacter.IsValid())
	{
		actorToIgnore.Add(TargetedCharacter.Get());
	}
	UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(), startLocation, endLocation, Radius, "Pawn", false
		, actorToIgnore, EDrawDebugTrace::None, hitResultList, true);

	float minDist = FLT_MAX;
	ACharacterBase* target = nullptr;
	for (FHitResult& result : hitResultList)
	{
		if (!result.bBlockingHit) continue;

		AActor* pawn = result.GetActor();

		if (!IsValid(pawn)) continue;
		if (!pawn->Tags.IsValidIndex(1) || !OwnerCharacter.Get()->Tags.IsValidIndex(1)) continue;
		if (pawn->Tags[1] == OwnerCharacter.Get()->Tags[1]) continue;

		float dist = FVector::Distance(pawn->GetActorLocation(), startLocation);
		if (dist < minDist)
		{
			dist = minDist;
			target = Cast<ACharacterBase>(pawn);
		}
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
	TargetedCharacter = TargetedCandidate;

	bIsTargetingActivated = true;
	GetOwner()->GetWorldTimerManager().ClearTimer(CameraRotateTimerHandle);
	GetOwner()->GetWorldTimerManager().SetTimer(CameraRotateTimerHandle, this, &UTargetingManagerComponent::UpdateCameraRotation, 0.01f, true);

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
		FollowingCameraRef.Get()->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator, false);
	}
	OnTargetingActivated.Broadcast(false, nullptr);
}

void UTargetingManagerComponent::UpdateTargetedCandidate()
{
	AActor* newCandidate = FindNearEnemyToTarget();
	if (IsValid(newCandidate))
	{
		TargetedCandidate = Cast<ACharacterBase>(newCandidate);
		OnTargetUpdated.Broadcast(TargetedCandidate.Get());
	}

	if (TargetedCandidate.IsValid())
	{
		if (FVector::Dist(OwnerCharacter.Get()->GetActorLocation(), TargetedCandidate.Get()->GetActorLocation()) > MaxFindDistance)
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

	if (FVector::Dist(OwnerCharacter.Get()->GetActorLocation(), TargetedCandidate.Get()->GetActorLocation()) >= 50.0f)
	{
		FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(FollowingCameraRef.Get()->GetComponentLocation(), TargetedCharacter.Get()->GetActorLocation());
		newRotation.Roll = 0.0f;
		OwnerCharacter.Get()->GetController()->SetControlRotation(newRotation);
	}
}

ACharacterBase* UTargetingManagerComponent::GetTargetedCharacter()
{
	if (TargetedCharacter.IsValid() && !TargetedCharacter.IsExplicitlyNull())
	{
		return TargetedCharacter.Get();
	}
	return nullptr;
}

void UTargetingManagerComponent::SetTargetedCharacter(ACharacterBase* NewTarget)
{
	TargetedCharacter = NewTarget;
}

