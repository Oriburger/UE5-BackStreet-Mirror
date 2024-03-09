// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/ThrowWeaponBase.h"
#include "../public/ProjectileBase.h"
#include "../public/RangedWeaponBase.h"
#include "../public/WeaponInventoryBase.h"
#include "../../Character/public/CharacterBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#define AUTO_RELOAD_DELAY_VALUE 0.1
#define MAX_THROW_DISTANCE 1200.0f //AMainCharacterBase와 통일 (추후 하나의 파일로 통합 예정)	
#define THROW_DELAY_VALUE 1.0f

AThrowWeaponBase::AThrowWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	this->Tags.Add("Throw");

	ProjectilePathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Projectile Path"));
	ProjectilePathSpline->SetupAttachment(RootComponent);
	ProjectilePathSpline->bHiddenInGame = false; 
	ProjectilePathSpline->SetVisibility(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> splineMeshFinder(TEXT("/Game/Weapon/StaticMesh/SM_SplineMesh.SM_SplineMesh"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> splineMeshMaterialFinder(TEXT("/Game/Weapon/Material/M_ProjectilePathSpline.M_ProjectilePathSpline"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> pathTargetDecalMaterialFinder(TEXT("/Game/Weapon/Material/M_ProjectileTarget.M_ProjectileTarget"));
	checkf(splineMeshFinder.Succeeded(), TEXT("spline mesh 클래스 탐색에 실패했습니다."));
	checkf(splineMeshMaterialFinder.Succeeded(), TEXT("spline mesh material 클래스 탐색에 실패했습니다."));
	checkf(pathTargetDecalMaterialFinder.Succeeded(), TEXT("path target decal material 클래스 탐색에 실패했습니다."));
	
	SplineMesh = splineMeshFinder.Object;
	SplineMeshMaterial = splineMeshMaterialFinder.Object;
	PathTargetDecalMaterial = pathTargetDecalMaterialFinder.Object;
}

void AThrowWeaponBase::Attack()
{
	Super::Attack();
}

void AThrowWeaponBase::StopAttack()
{
	Super::StopAttack();
}

float AThrowWeaponBase::GetAttackRange()
{
	return 1000.0f;
}

bool AThrowWeaponBase::TryFireProjectile()
{
	return Super::TryFireProjectile();
}

void AThrowWeaponBase::Reload()
{
	return Super::Reload();
}

void AThrowWeaponBase::AddAmmo(int32 Count)
{
	Super::AddAmmo(Count);
}

void AThrowWeaponBase::Throw()
{
	if (!IsValid(GetOwner()) || !GetOwner()->ActorHasTag("Character")) return;
	if (!GetCanThrow()) return;

	//Erase spline
	ClearProjectilePathSpline();

	//발사체를 생성하고 투척
	ACharacterBase* ownerCharacter = Cast<ACharacterBase>(GetOwner());
	AProjectileBase* newProjectile = CreateProjectile();

	if (IsValid(newProjectile))
	{
		newProjectile->ProjectileMovement->Velocity = ThrowDirection;
		newProjectile->ProjectileMovement->InitialSpeed = ThrowSpeed;
		newProjectile->ProjectileMovement->Activate();

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShootNiagaraEmitter
													, newProjectile->GetActorLocation(), ThrowDirection.Rotation());

		WeaponState.RangedWeaponState.CurrentAmmoCount -= 1;
		OwnerCharacterRef.Get()->GetSubInventoryRef()->SyncCurrentWeaponInfo(true);
	}

	if (IsValid(ShootSound))
	{
		PlaySingleSound(ShootSound, "Shoot");
	}

	//탄환이 다 되었다면? 자동으로 제거
	if (WeaponState.RangedWeaponState.CurrentAmmoCount == 0 && WeaponState.RangedWeaponState.ExtraAmmoCount == 0)
	{
		//플레이어가 소유한 보조무기라면? 보조무기 인벤토리에서 제거
		GetWorldTimerManager().ClearTimer(ThrowDelayHandle);
		ownerCharacter->DropWeapon();
	}

	//딜레이 타이머를 생성
	UE_LOG(LogTemp, Warning, TEXT("CREATE DELAY TIMER"));
	GetWorldTimerManager().SetTimer(ThrowDelayHandle, THROW_DELAY_VALUE, false);
}

FPredictProjectilePathResult AThrowWeaponBase::GetProjectilePathPredictResult()
{
	if (!IsValid(GetOwner())) return FPredictProjectilePathResult();
	if (!GetOwner()->ActorHasTag("Character")) return FPredictProjectilePathResult();

	ACharacterBase* ownerCharacter = Cast<ACharacterBase>(GetOwner());

	FVector startLocation = ownerCharacter->GetMesh()->GetSocketLocation("weapon_r");

	FPredictProjectilePathResult predictProjectilePathResult;
	FPredictProjectilePathParams predictProjectilePathParams(1.0f, startLocation, ThrowDirection, 2.0f);
	predictProjectilePathParams.ActorsToIgnore = { this, ownerCharacter };

	UGameplayStatics::PredictProjectilePath(GetWorld(), predictProjectilePathParams, predictProjectilePathResult);

	return predictProjectilePathResult;
}

void AThrowWeaponBase::CalculateThrowDirection(FVector NewDestination)
{
	if (!IsValid(GetOwner())) return;
	if (!GetOwner()->ActorHasTag("Character")) return;
	if (ProjectileStat.ProjectileID == 0)
	{
		ProjectileStat = GetProjectileStatInfo(WeaponAssetInfo.RangedWeaponAssetInfo.ProjectileID);
		ThrowSpeed = ProjectileStat.ProjectileSpeed;
	}

	ACharacterBase* ownerCharacter = Cast<ACharacterBase>(GetOwner());

	ThrowDestination = NewDestination;

	//DrawDebugSphere(GetWorld(), ThrowDestination, 8.0f, 12, FColor::Red, false, 0.025, 0, 5.0f);

	ThrowDirection = FVector(0.0f);
	const float initialSpeed = FMath::Clamp(ProjectileStat.ProjectileSpeed, 900.0f, 2000.0f);
	const bool bIsHighArc = false; // UKismetMathLibrary::Vector_Distance(GetActorLocation(), ThrowDestination) >= (MAX_THROW_DISTANCE);
	ThrowSpeed = initialSpeed;

	UGameplayStatics::SuggestProjectileVelocity(GetWorld(), ThrowDirection
			, ownerCharacter->GetMesh()->GetSocketLocation("weapon_r"), ThrowDestination
			, ThrowSpeed, bIsHighArc, 0.25f, 0.0f, ESuggestProjVelocityTraceOption::DoNotTrace
			, FCollisionResponseParams::DefaultResponseParam, { GetOwner(), this }, false);
}

void AThrowWeaponBase::UpdateProjectilePathSpline(TArray<FVector>& Locations)
{
	ClearProjectilePathSpline();

	//AMainCharacter call this function.
	//use the result of "AThrowWeaponBase::GetProjectilePathPredictResult()"
	ProjectilePathSpline->SetSplinePoints(Locations, ESplineCoordinateSpace::Local);
	for (int idx = 1; idx < Locations.Num(); idx++)
	{
		FVector prevPos, prevTangent;
		FVector currPos, currTangent;
		ProjectilePathSpline->GetLocationAndTangentAtSplinePoint(idx - 1, prevPos, prevTangent, ESplineCoordinateSpace::Local);
		ProjectilePathSpline->GetLocationAndTangentAtSplinePoint(idx, currPos, currTangent, ESplineCoordinateSpace::Local);

		//spawn spline mesh component
		USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(ProjectilePathSpline, USplineMeshComponent::StaticClass());
		splineMesh->SetForwardAxis(ESplineMeshAxis::Z);
		if(SplineMesh) splineMesh->SetStaticMesh(SplineMesh);

		//init spline mesh's mobility 
		splineMesh->SetMobility(EComponentMobility::Movable);
		splineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;

		//register spline mesh component to world
		splineMesh->RegisterComponentWithWorld(GetWorld());

		//attach new splineMesh to spline component
		splineMesh->AttachToComponent(ProjectilePathSpline, FAttachmentTransformRules::KeepWorldTransform);
		splineMesh->SetStartScale(FVector2D(0.25f));
		splineMesh->SetEndScale(FVector2D(0.25f));

		//set spline mesh's position and collision
		splineMesh->SetStartAndEnd(prevPos, prevTangent, currPos, currTangent);
		splineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (SplineMeshMaterial) splineMesh->SetMaterial(0, SplineMeshMaterial);
		SplineMeshComponentList.Add(splineMesh);
	}
	//DrawDebugSphere(GetWorld(), Locations[Locations.Num() - 1], 10.0f, 5, FColor::Yellow, false, 1.0f, 1, 0);
	UGameplayStatics::SpawnDecalAtLocation(GetWorld(), PathTargetDecalMaterial, FVector(20.0f, 100.0f, 100.0f), Locations[Locations.Num() - 1], FRotator(90.0f, 0.0f, 0.0f), 0.01f);
}

void AThrowWeaponBase::ClearProjectilePathSpline()
{
	for (int idx = SplineMeshComponentList.Num() - 1; idx >= 0; idx--)
	{
		SplineMeshComponentList[idx]->DestroyComponent();
	}
	SplineMeshComponentList.Empty();
}

float AThrowWeaponBase::GetThrowDelayRemainingTime()
{
	if (!GetWorldTimerManager().IsTimerActive(ThrowDelayHandle)) return 0.0f;
	return GetWorldTimerManager().GetTimerRemaining(ThrowDelayHandle);;
}

