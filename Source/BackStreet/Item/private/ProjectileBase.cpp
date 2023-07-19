// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/ProjectileBase.h"
#include "../public/WeaponBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Global/public/DebuffManager.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SPHERE_COLLISION"));
	SphereCollision->SetCollisionProfileName(TEXT("OverlapAll"));
	
	TargetingCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TARGETING_COLLISION"));
	TargetingCollision->SetupAttachment(RootComponent);
	TargetingCollision->SetWorldScale3D(FVector(5.0f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PROJECTILE_MESH"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetRelativeScale3D({ 0.5f, 0.5f, 0.5f });
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	 
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PROJECTILE_MOVEMENT"));
	ProjectileMovement->InitialSpeed = ProjectileStat.ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileStat.ProjectileSpeed;
	ProjectileMovement->bAutoActivate = false;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnProjectileBeginOverlap);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnTargetBeginOverlap);

	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}

void AProjectileBase::InitProjectile(ACharacterBase* NewCharacterRef)
{
	if (IsValid(NewCharacterRef))
	{
		GamemodeRef->UpdateProjectileStatWithID(this, ProjectileID);

		OwnerCharacterRef = NewCharacterRef;
		SpawnInstigator = OwnerCharacterRef->GetController();

		ProjectileMovement->InitialSpeed = ProjectileStat.ProjectileSpeed;
		ProjectileMovement->MaxSpeed = ProjectileStat.ProjectileSpeed;
	}
}

void AProjectileBase::UpdateProjectileStat(FProjectileStatStruct NewStat)
{
	ProjectileStat = NewStat;
}

void AProjectileBase::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
	, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!ProjectileMovement->IsActive() || (IsValid(GetOwner()) && OtherActor == GetOwner())) return;
	if (OtherActor->ActorHasTag("Item")) return;

	if (OtherActor->ActorHasTag("Character"))
	{
		if (OtherActor == OwnerCharacterRef || OtherActor->ActorHasTag(OwnerCharacterRef->Tags[1])) return;

		//폭발하는 발사체라면?
		if (ProjectileStat.bIsExplosive)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), ProjectileStat.ProjectileDamage, ProjectileStat.ProjectileDamage / 2.0f
														, SweepResult.Location, 100.0f, 200.0f, 25.0f, nullptr, {}, OwnerCharacterRef, OwnerCharacterRef->Controller);
			GamemodeRef->PlayCameraShakeEffect(ECameraShakeType::E_Explosion, SweepResult.Location, 100.0f);
			
			if (ExplosionSound != nullptr)
			{
				const float soundVolume = OwnerCharacterRef->ActorHasTag("Player") ? 1.0f : 0.2f;
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation(), soundVolume);
			}
		}
		else
		{
			const float totalDamage = ProjectileStat.ProjectileDamage * OwnerCharacterRef->GetCharacterStat().CharacterAtkMultiplier;
			UGameplayStatics::ApplyDamage(OtherActor, totalDamage,SpawnInstigator, OwnerCharacterRef, nullptr);
			GamemodeRef->PlayCameraShakeEffect(ECameraShakeType::E_Hit, SweepResult.Location, 100.0f);
		}
		GamemodeRef->GetGlobalDebuffManagerRef()->SetDebuffTimer(ProjectileStat.DebuffType, Cast<ACharacterBase>(OtherActor), OwnerCharacterRef
																, ProjectileStat.DebuffTotalTime, ProjectileStat.DebuffVariable);
	}

	FTransform TargetTransform = { FRotator(), SweepResult.Location, {1.0f, 1.0f, 1.0f} };
	if (HitSound != nullptr && HitParticle != nullptr)
	{
		const float soundVolume = OwnerCharacterRef->ActorHasTag("Player") ? 1.0f : 0.2;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, TargetTransform);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation(), soundVolume);
	}
	Destroy();
}

void AProjectileBase::OnTargetBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !IsValid(OwnerCharacterRef)) return;
	if ( OtherActor->ActorHasTag(OwnerCharacterRef->Tags[1])) return;
	if (!ProjectileStat.bIsHoming || OtherActor == OwnerCharacterRef || !bIsActivated) return;

	ProjectileMovement->HomingTargetComponent = OverlappedComp;
	ProjectileMovement->bIsHomingProjectile = true;
}

void AProjectileBase::ActivateProjectileMovement()
{
	ProjectileMovement->Activate();
	bIsActivated = true; 
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

