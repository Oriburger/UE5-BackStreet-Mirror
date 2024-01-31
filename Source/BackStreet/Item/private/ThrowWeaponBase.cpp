// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/ThrowWeaponBase.h"
#include "../public/ProjectileBase.h"
#include "../public/RangedWeaponBase.h"
#include "../public/WeaponInventoryBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Global/public/DebuffManager.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
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

bool AThrowWeaponBase::TryReload()
{
	return Super::TryReload();
}

void AThrowWeaponBase::AddAmmo(int32 Count)
{
	Super::AddAmmo(Count);
}

void AThrowWeaponBase::Throw()
{
	if (!IsValid(GetOwner()) || !GetOwner()->ActorHasTag("Character")) return;
	if (!GetCanThrow()) return;

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
	}

	ACharacterBase* ownerCharacter = Cast<ACharacterBase>(GetOwner());

	ThrowDestination = NewDestination;

	ThrowDirection = FVector(0.0f);
	const float initialSpeed = FMath::Clamp(ProjectileStat.ProjectileSpeed, 900.0f, 2000.0f);
	const bool bIsHighArc = UKismetMathLibrary::Vector_Distance(GetActorLocation(), ThrowDestination) >= (MAX_THROW_DISTANCE / 2.0f);
	const float highArcSpeedMultipiler = bIsHighArc ? 1.2f : 1.0f;
	ThrowSpeed = initialSpeed * highArcSpeedMultipiler;

	UGameplayStatics::SuggestProjectileVelocity(GetWorld(), ThrowDirection
			, ownerCharacter->GetMesh()->GetSocketLocation("weapon_r"), ThrowDestination
			, initialSpeed * highArcSpeedMultipiler, bIsHighArc, 0.25f, 0.0f, ESuggestProjVelocityTraceOption::DoNotTrace
			, FCollisionResponseParams::DefaultResponseParam, { GetOwner(), this }, false);
}

float AThrowWeaponBase::GetThrowDelayRemainingTime()
{
	if (!GetWorldTimerManager().IsTimerActive(ThrowDelayHandle)) return 0.0f;
	return GetWorldTimerManager().GetTimerRemaining(ThrowDelayHandle);;
}

