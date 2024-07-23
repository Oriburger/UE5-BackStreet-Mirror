// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Character/Component/WeaponComponentBase.h"
#include "../../../System/AssetSystem/AssetManagerBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickInterval(0.01f);

	RootComponent = SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SPHERE_COLLISION"));
	SphereCollision->SetCollisionProfileName(TEXT("Projectile"));
	SphereCollision->SetNotifyRigidBodyCollision(true);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	TargetingCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TARGETING_COLLISION"));
	TargetingCollision->SetupAttachment(RootComponent);
	TargetingCollision->SetWorldScale3D(FVector(5.0f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PROJECTILE_MESH"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetWorldScale3D({ 0.5f, 0.5f, 0.5f });
	Mesh->SetRelativeRotation(FRotator(0.0f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionProfileName("Item");
	Mesh->SetNotifyRigidBodyCollision(true);	

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PROJECTILE_MOVEMENT"));
	ProjectileMovement->InitialSpeed = ProjectileStat.ProjectileSpeed;
	ProjectileMovement->bAutoActivate = false;

	TrailParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TRAIL_PARTICLE"));
	TrailParticle->SetupAttachment(Mesh);
	TrailParticle->SetRelativeLocation(FVector(0.0f));
	TrailParticle->bAutoActivate = false;

	InitialLifeSpan = 10.0f;
	this->Tags.Add("Projectile");
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();	
	
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnProjectileBeginOverlap);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnTargetBeginOverlap);
	SphereCollision->OnComponentHit.AddDynamic(this, &AProjectileBase::OnProjectileHit);

	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}

void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//총알형 발사체가 아니라면, 날아갈때 로테이션을 조금씩 돌려줌
	if(ProjectileMovement->IsActive() && !ProjectileStat.bIsBullet)
		Mesh->SetRelativeRotation(ProjectileMovement->Velocity.Rotation());
}

void AProjectileBase::InitProjectileAsset()
{
	if (ProjectileAssetInfo.ProjectileMesh.IsValid())
	{
		Mesh->SetStaticMesh(ProjectileAssetInfo.ProjectileMesh.Get());
		Mesh->SetRelativeLocation(ProjectileAssetInfo.InitialLocation);
		Mesh->SetWorldScale3D(ProjectileAssetInfo.InitialScale);
	}

	if (ProjectileAssetInfo.HitEffectParticle.IsValid())
		HitNiagaraParticle = ProjectileAssetInfo.HitEffectParticle.Get();

	if (ProjectileAssetInfo.HitEffectParticleLegacy.IsValid())
		HitParticle = ProjectileAssetInfo.HitEffectParticleLegacy.Get();

	if (ProjectileAssetInfo.ExplosionParticle.IsValid())
		ExplosionParticle = ProjectileAssetInfo.ExplosionParticle.Get();

	if (ProjectileAssetInfo.TrailParticle.IsValid())
		TrailParticle->SetAsset(ProjectileAssetInfo.TrailParticle.Get());
}

void AProjectileBase::DestroyWithEffect(FVector Location, bool bContainCameraShake)
{
	if (bContainCameraShake)
	{
		GamemodeRef.Get()->PlayCameraShakeEffect(ECameraShakeType::E_Hit, Location, 100.0f);
	}

	FTransform targetTransform = { FRotator::ZeroRotator, Location, {1.0f, 1.0f, 1.0f} };
	
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Weapon, floor(ProjectileID/10.0)*10, "HitImpact");
	}

	if (HitParticle != nullptr)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, targetTransform);

	if (HitNiagaraParticle != nullptr)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitNiagaraParticle, targetTransform.GetLocation(), targetTransform.GetRotation().Rotator());

	Destroy();
}

void AProjectileBase::InitProjectile(ACharacterBase* NewCharacterRef, FProjectileAssetInfoStruct NewAssetInfo, FProjectileStatStruct NewStatInfo)
{
	if (IsValid(NewCharacterRef))
	{
		OwnerCharacterRef = NewCharacterRef;
		SpawnInstigator = OwnerCharacterRef->GetController();
	}
	ProjectileID = NewAssetInfo.ProjectileID;
	UpdateProjectileStat(NewStatInfo);

	AssetManagerBaseRef = GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();

	ProjectileAssetInfo = NewAssetInfo;

	if (!ProjectileStat.bIsBullet)
		SetActorRotation(UKismetMathLibrary::RandomRotator());

	if (ProjectileID != 0)
	{
		TArray<FSoftObjectPath> tempStream, assetToStream;
		tempStream.AddUnique(ProjectileAssetInfo.ProjectileMesh.ToSoftObjectPath());
		tempStream.AddUnique(ProjectileAssetInfo.TrailParticle.ToSoftObjectPath());
		tempStream.AddUnique(ProjectileAssetInfo.HitEffectParticle.ToSoftObjectPath());
		tempStream.AddUnique(ProjectileAssetInfo.HitEffectParticleLegacy.ToSoftObjectPath());
			
		for (auto& assetPath : tempStream)
		{
			if (!assetPath.IsValid() || assetPath.IsNull()) continue;
			assetToStream.AddUnique(assetPath);
		}
		FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
		streamable.RequestAsyncLoad(assetToStream, FStreamableDelegate::CreateUObject(this, &AProjectileBase::InitProjectileAsset));
	}
	else Mesh->SetStaticMesh(nullptr);
}

void AProjectileBase::UpdateProjectileStat(FProjectileStatStruct NewStat)
{
	ProjectileStat = NewStat;
	ProjectileMovement->bIsHomingProjectile = ProjectileStat.bIsHoming; 
	ProjectileMovement->ProjectileGravityScale =  ProjectileStat.bIsBullet ? 0.0f : 1.0f;
	ProjectileMovement->InitialSpeed = ProjectileStat.ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileStat.ProjectileSpeed;
	ProjectileMovement->Velocity = GetActorRotation().Vector().GetSafeNormal() * ProjectileStat.ProjectileSpeed;
}

void AProjectileBase::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
	, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!ProjectileMovement->IsActive()) return;
	if (!OwnerCharacterRef.IsValid() || !GamemodeRef.IsValid()) return;
	if (!IsValid(OtherActor) || OtherActor->ActorHasTag("Item")) return;

	if (OtherActor->ActorHasTag("Character"))
	{
		//It would be change later
		if(!ProjectileState.bCanAttackCauser&& (OtherActor->ActorHasTag(OwnerCharacterRef.Get()->Tags[1]))) return;

		//디버프가 있다면?
		Cast<ACharacterBase>(OtherActor)->TryAddNewDebuff(ProjectileStat.DebuffInfo, OwnerCharacterRef.Get());

		//폭발하는 발사체라면?
		if (ProjectileStat.bIsExplosive)
		{
			Explode();
			return;
		}
		else
		{
			const float totalDamage = Cast<ACharacterBase>(GetOwner())->WeaponComponent->CalculateTotalDamage(Cast<ACharacterBase>(OtherActor)->GetCharacterState());
			UGameplayStatics::ApplyDamage(OtherActor, totalDamage, SpawnInstigator, OwnerCharacterRef.Get(), nullptr);
			DestroyWithEffect(SweepResult.Location, !Cast<ACharacterBase>(OtherActor)->GetCharacterStat().bIsInvincibility);
		}
	}
}

void AProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponet, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//발사체가 총알일경우, 즉시 제거 
	if (ProjectileStat.bIsBullet)
	{
		if (ProjectileStat.bIsExplosive) Explode();
		DestroyWithEffect(GetActorLocation(), false);
		return;	
	}

	//메시 콜리전으로만 바닥과 상호작용 하도록 지정
	SphereCollision->SetRelativeScale3D(FVector(0.1f));
	Mesh->SetWorldScale3D(ProjectileAssetInfo.InitialScale);

	//튀는 현상을 줄이기 위한 댐핑 지정
	SphereCollision->SetLinearDamping(0.5f);
	SphereCollision->SetAngularDamping(0.5f);

	//피직스를 설정하고 구르도록 임펄스를 제공
	SphereCollision->SetSimulatePhysics(true);
	SphereCollision->AddAngularImpulseInDegrees(ProjectileMovement->Velocity);

	//폭발이 가능한 발사체라면?
	if(ProjectileStat.bIsExplosive)
		GetWorldTimerManager().SetTimer(AutoExplodeTimer, this, &AProjectileBase::Explode, 1.5f, false);
}

void AProjectileBase::OnTargetBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{		
	if (!IsValid(OtherActor) || !OwnerCharacterRef.IsValid()) return;
	if ( OtherActor->ActorHasTag(OwnerCharacterRef.Get()->Tags[1])) return;
	if (!ProjectileStat.bIsHoming || OtherActor == OwnerCharacterRef.Get() || !bIsActivated) return;

	ProjectileMovement->HomingTargetComponent = OverlappedComp;
	ProjectileMovement->bIsHomingProjectile = true;
}

void AProjectileBase::Explode()
{
	if (IsActorBeingDestroyed() || !IsValid(this)) return; 
	if (!GamemodeRef.IsValid() || !OwnerCharacterRef.IsValid()) return; 
	if (!ProjectileStat.bIsExplosive) return;

	//--- 데미지 관련 --------------
	TArray<AActor*> ignoreActors = { (AActor*)this };
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), ProjectileStat.ProjectileDamage, ProjectileStat.ProjectileDamage * 0.2f
		, GetActorLocation(), 125.0f, 250.0f, 0.5f, nullptr, ignoreActors, OwnerCharacterRef.Get()
		, OwnerCharacterRef.Get()->GetController(), ECC_WorldStatic);

	//--- 이펙트 출력 --------------
	if (ExplosionParticle != nullptr)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionParticle, GetActorLocation(), GetActorRotation());
	
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Weapon, floor(ProjectileID / 10.0) * 10, "Explosion");
	}

	GamemodeRef.Get()->PlayCameraShakeEffect(ECameraShakeType::E_Explosion, GetActorLocation(), 5000.0f); //카메라 셰이크 이벤트

	Destroy();
}

void AProjectileBase::SetOwnerCharacter(ACharacterBase* NewOwnerCharacterRef)
{
	if (!IsValid(NewOwnerCharacterRef)) return;
	OwnerCharacterRef = NewOwnerCharacterRef;
	SetOwner(OwnerCharacterRef.Get());
}

void AProjectileBase::ActivateProjectileMovement()
{
	ProjectileMovement->Activate();
	if (IsValid(TrailParticle))
	{
		TrailParticle->bAutoActivate = true;
	}
	bIsActivated = true; 
}
