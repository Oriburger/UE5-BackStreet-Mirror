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
	float finalTraceRadius = TraceRadius;

	//�̻�� ����
	if (bIsTargetingActivated && TargetedCharacter.IsValid())
	{
		actorToIgnore.Add(TargetedCharacter.Get());
	}

	
	//���� ���϶� (�� ��������)
	if (!OwnerCharacter.Get()->ActionTrackingComponent->GetIsActionReady("Attack")
		|| !OwnerCharacter.Get()->ActionTrackingComponent->GetIsActionReady("JumpAttack")
		|| !OwnerCharacter.Get()->ActionTrackingComponent->GetIsActionReady("DashAttack"))
	{
		//��	�� ���ϋ����� Ʈ���̽� ������	���̰�, ������ Ÿ���õ� ĳ������ �������� Ʈ���̽��Ѵ�.
		// Ÿ������ Ȱ��ȭ�Ǿ��ְ�, Ÿ���� �ִٸ� �� Ÿ���� �������� Ʈ���̽��Ѵ� ���ٸ� �޽��� �չ������� Ʈ���̽�
		traceDirection = TargetedCandidate.IsValid() ? TargetedCandidate.Get()->GetActorForwardVector() * -1.0f :
			OwnerCharacter.Get()->GetMesh()->GetRightVector();
		startLocation = OwnerCharacter.Get()->GetActorLocation() + traceDirection * 25.0f;
		endLocation = startLocation + traceDirection * MaxFindDistance;

		//(Player Only) �̵� �Է� ���϶�  �̵� �Է��� ���� ���� �� �������� Ʈ���̽��Ѵ�.
		FVector2D movementInputValue = IsValid(Cast<AMainCharacterBase>(OwnerCharacter.Get())) ? Cast<AMainCharacterBase>(OwnerCharacter.Get())->GetCurrentMovementInputValue() : FVector2D::ZeroVector;
		if (movementInputValue.Length() > 0)
		{
			//turnAngle�� �°� traceDirection�� ����Ѵ�. ������ Mesh RightVector
			float turnAngle = FMath::RadiansToDegrees(FMath::Atan2(movementInputValue.X, movementInputValue.Y));
			traceDirection = UKismetMathLibrary::RotateAngleAxis(FollowingCameraRef.Get()->GetForwardVector(), turnAngle, FVector(0.0f, 0.0f, 1.0f));
			
			traceDirection.Normalize(); traceDirection.Z = 0.0f; //Z���� �����Ѵ�.
			endLocation = startLocation + traceDirection * MaxFindDistance;
			
			//draw debug info
			if (bShowDebugSphere)
			{
				//draw movement input value
				UKismetSystemLibrary::DrawDebugArrow(GetWorld(), startLocation, endLocation, 50.0f, FColor::Red, AutoTargetingRate, 1.0f);
				UKismetSystemLibrary::DrawDebugSphere(GetWorld(), startLocation, 10.0f, 12, FColor::Green, AutoTargetingRate, 1.0f);
			}
		}
	}

	// ==== Ʈ���̽��� ���� ���̳� ��ֹ��� �ִ��� Ȯ���Ѵ� ================================
	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectType;
	traceObjectType.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	traceObjectType.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	FHitResult hitResult;
	bool bIsHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), OwnerCharacter.Get()->GetActorLocation(), endLocation, (RadiusOverride <= 1.0f ? finalTraceRadius : RadiusOverride) / 3.0f,
		traceObjectType, true, actorToIgnore, bShowDebugSphere ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitResult, true, FColor::Yellow, FColor::Blue, AutoTargetingRate);
	if (bIsHit)
	{
		endLocation = hitResult.Location;
	}

	//trace ����
	if (!bUseConeTrace)
	{
		UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(), startLocation, endLocation, RadiusOverride <= 1.0f ? finalTraceRadius : RadiusOverride, "Pawn", true
			, actorToIgnore, bShowDebugSphere ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitResultList, true, FColor::Green, FColor::Red, AutoTargetingRate);
	}
	else
	{
		ConeTraceByProfile(startLocation, endLocation, RadiusOverride <= 1.0f ? finalTraceRadius : RadiusOverride, "Pawn", true
			, actorToIgnore, bShowDebugSphere ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitResultList, true, FColor::Green, FColor::Red, AutoTargetingRate);
	}
	

	float minDist = FLT_MAX;
	ACharacterBase* target = nullptr;
	
	bool bContainsOldCandidate = false; //Ÿ������ �����ְ� �� Ʈ���̽��� �����ٸ�?
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
			ACharacterBase* characterRef = Cast<ACharacterBase>(pawn);
			if (IsValid(characterRef) && characterRef->GetIsActionActive(ECharacterActionType::E_Die)) continue;
			target = Cast<ACharacterBase>(characterRef);
		}
	}

	//������ �Ǿ��ִٸ� �׳� �� ���� ��ȯ
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

bool UTargetingManagerComponent::ActivateTargeting()
{
	if (!TargetedCandidate.IsValid()) return false;

	if (bIsTargetingActivated || TargetedCharacter == TargetedCandidate) return false;
	
	SetTargetedCharacter(TargetedCandidate.Get());
	
	bIsTargetingActivated = true;
	GetOwner()->GetWorldTimerManager().ClearTimer(CameraRotateTimerHandle);
	GetOwner()->GetWorldTimerManager().SetTimer(CameraRotateTimerHandle, this, &UTargetingManagerComponent::UpdateCameraRotation, 0.01f, true);

	TargetedCharacter.Get()->GetCharacterMovement()->bOrientRotationToMovement = false;
	TargetedCharacter.Get()->GetCharacterMovement()->bUseControllerDesiredRotation = true;

	OnTargetingActivated.Broadcast(true, TargetedCharacter.Get());
	return true;
}

void UTargetingManagerComponent::DeactivateTargeting()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
		FString::Printf(TEXT("Deactivate Targeting")));

	TargetedCharacter.Reset();
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

	//�ĺ� ������Ʈ�� ������ ���� �����϶� �����Ѵ� 
	//�⺻������ newCandidate�� valid�� �� (���������� ã������)
	//���� �������� �ƴҶ� Ȥ�� �ƹ��� ������
	TargetedCandidate = Cast<ACharacterBase>(newCandidate);

	if (TargetedCandidate.IsValid() && !OwnerCharacter.Get()->GetCharacterGameplayInfo().bIsAirAttacking)
	{
		if (FVector::Dist(OwnerCharacter.Get()->GetActorLocation(), TargetedCandidate.Get()->GetActorLocation()) > TargetingMaintainThreashold)
		{
			TargetedCandidate.Reset();
			DeactivateTargeting();
			return;
		}
	}
	OnTargetUpdated.Broadcast(TargetedCandidate.Get());
}

void UTargetingManagerComponent::UpdateCameraRotation()
{
	if (!FollowingCameraRef.IsValid() || !TargetedCharacter.IsValid() || !OwnerCharacter.IsValid()) return;
	if (OwnerCharacter.Get()->ActionTrackingComponent->GetIsActionInProgress("Skill")) return;
	if (OwnerCharacter.Get()->GetIsActionActive(ECharacterActionType::E_Die) 
		|| TargetedCharacter.Get()->GetIsActionActive(ECharacterActionType::E_Die)
		|| FVector::Dist(OwnerCharacter.Get()->GetActorLocation(), TargetedCharacter.Get()->GetActorLocation()) >= TargetingMaintainThreashold)
	{
		DeactivateTargeting();
		Cast<AMainCharacterBase>(OwnerCharacter.Get())->ResetCameraBoom();
		return;
	}
	if (!TargetedCharacter.Get()->GetMesh()->IsVisible() || TargetedCharacter.Get()->GetMesh()->bHiddenInGame) return;

	if (FVector::Dist(OwnerCharacter.Get()->GetActorLocation(), TargetedCharacter.Get()->GetActorLocation()) >= 50.0f)
	{
		FVector targetLocation = TargetedCharacter.Get()->GetActorLocation();
		targetLocation.Z = OwnerCharacter.Get()->GetActorLocation().Z; //Z���� �����ϰ� XY��鿡���� �ٶ󺻴�.
		FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(FollowingCameraRef.Get()->GetComponentLocation(), targetLocation);
		newRotation.Roll = 0.0f;
		OwnerCharacter.Get()->GetController()->SetControlRotation(newRotation);
	}
}

bool UTargetingManagerComponent::ConeTraceByProfile(
	const FVector Start, const FVector End, float Radius, FName ProfileName,
	bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
	EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits,
	bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	OutHits.Reset();

	// ���� ����
	const FVector forward = (End - Start).GetSafeNormal();

	// ȸ�� ���� ��: Z�� ���� Yaw ȸ�� (2D ����)
	const FRotator baseRot = forward.Rotation();

	// ���� ����
	const float halfAngleDeg = TraceAngle;
	const float minAngle = -halfAngleDeg;
	const float maxAngle = +halfAngleDeg;

	// Density �� Step �� ���� (ex: 0.5f �� 4�� ��)
	int32 numSteps = FMath::Clamp(FMath::RoundToInt(FMath::Lerp(1.f, 20.f, TraceDensity)), 1, 360);
	const float stepAngle = (maxAngle - minAngle) / numSteps;

	// ���� ����Ʈ ����
	TArray<float> anglesToTrace;
	anglesToTrace.AddUnique(minAngle);
	anglesToTrace.AddUnique(0.0f);
	anglesToTrace.AddUnique(maxAngle);

	for (int32 i = 1; i < numSteps; ++i)
	{
		float angle = minAngle + stepAngle * i;
		anglesToTrace.AddUnique(angle);
	}


	// Ʈ���̽� ����
	TArray<AActor*> duplicateCheckActors;
	for (float angle : anglesToTrace)
	{
		// ���� ȸ�� ����
		FRotator rotatedYaw = baseRot + FRotator(0, angle, 0);
		FVector dir = rotatedYaw.Vector();
		FVector traceEnd = Start + dir * (End - Start).Size();

		TArray<FHitResult> traceHits;
		UKismetSystemLibrary::SphereTraceMultiByProfile(
			this, Start, traceEnd, Radius, ProfileName, bTraceComplex, ActorsToIgnore,
			DrawDebugType, traceHits, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime
		);

		for (const FHitResult& hit : traceHits)
		{
			if (hit.bBlockingHit && IsValid(hit.GetActor()) && !duplicateCheckActors.Contains(hit.GetActor()))
			{
				// �ߺ� �˻�
				duplicateCheckActors.Add(hit.GetActor());
				OutHits.Add(hit);
			}
		}
	}

	return OutHits.Num() > 0;
}


ACharacterBase* UTargetingManagerComponent::GetTargetedCharacter()
{
	if (TargetedCharacter.IsValid() && !TargetedCharacter->GetIsActionActive(ECharacterActionType::E_Die))
	{
		return TargetedCharacter.Get();
	}
	return nullptr;
}

ACharacterBase* UTargetingManagerComponent::GetTargetedCandidate()
{
	if (TargetedCandidate.IsValid())
	{
		if (TargetedCandidate->GetIsActionActive(ECharacterActionType::E_Die))
		{
			TargetedCandidate.Reset();
			return nullptr;
		}
		return TargetedCandidate.Get();
	}
	return nullptr; 
}

void UTargetingManagerComponent::SetTargetedCharacter(ACharacterBase* NewTarget)
{
	TargetedCharacter = NewTarget;
}

bool UTargetingManagerComponent::GetIsEnemyInBoundary(FVector EnemyLocation)
{
	if (!OwnerCharacter.IsValid()) return false;

	// �÷��̾� ��ġ �� ���� ����
	FVector playerLocation = OwnerCharacter->GetActorLocation();
	FVector forwardVector = OwnerCharacter->GetMesh()->GetRightVector();
	FVector toEnemy = (EnemyLocation - playerLocation).GetSafeNormal();

	// ���� �������� �������� ���Ϳ��� ���� ���
	float angleBetween = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(forwardVector, toEnemy)));

	// ���� ���� ���� (�⺻ 15��, bUseConeTrace�� true�� TraceAngle ���)
	float angleLimit = bUseConeTrace ? TraceAngle : 15.0f;

	// �Ÿ� ���� �߰�
	bool isWithinDistance = FVector::Dist(playerLocation, EnemyLocation) <= MaxFindDistance;

	// ���� �Ǻ�
	bool isInSightCone = angleBetween <= angleLimit && isWithinDistance;

	UE_LOG(LogTemp, Warning, TEXT("GetIsEnemyInBoundary: %d (Angle: %.2f)"), isInSightCone, angleBetween);

	// �ð�ȭ
	FColor debugColor = isInSightCone ? FColor::Green : FColor::Red;
	UKismetSystemLibrary::DrawDebugSphere(GetWorld(), EnemyLocation, TraceRadius, 12, debugColor, 0.1f, 1.0f);

	return isInSightCone;
}