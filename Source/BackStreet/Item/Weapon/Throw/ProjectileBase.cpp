// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Character/Component/WeaponComponentBase.h"
#include "../../../Character/Component/DebuffManagerComponent.h"
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
	SphereCollision->SetNotifyRigidBodyCollision(false);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereCollision->SetSphereRadius(18.0f, false);

	TargetingCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TARGETING_COLLISION"));
	TargetingCollision->SetupAttachment(RootComponent);
	TargetingCollision->SetWorldScale3D(FVector(0.01f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PROJECTILE_MESH"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetWorldScale3D({ 0.5f, 0.5f, 0.5f });
	Mesh->SetRelativeRotation(FRotator(0.0f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionProfileName("Item");
	Mesh->SetNotifyRigidBodyCollision(false);	

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PROJECTILE_MOVEMENT"));
	ProjectileMovement->InitialSpeed = ProjectileStat.ProjectileSpeed;
	ProjectileMovement->bAutoActivate = false;

	TrailParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TRAIL_PARTICLE"));
	TrailParticle->SetupAttachment(Mesh);
	TrailParticle->SetRelativeLocation(FVector(0.0f));
	TrailParticle->bAutoActivate = false;

	InitialLifeSpan = ProjectileStat.LifeTime;
	this->Tags.Add("Projectile");
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();	

	if (!IsValid(GetInstigator()))
	{
		UE_LOG(LogTemp, Warning, TEXT("AProjectileBase::BeginPlay - Invalid Instigator or Controller or Pawn"));
		return;
	}
	SetOwnerCharacter(Cast<ACharacterBase>(GetInstigator()));

	// Homing
	TargetingCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnTargetBeginOverlap);

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnProjectileBeginOverlap);

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
		Mesh->SetRelativeRotation(ProjectileAssetInfo.InitialRotation);
		Mesh->SetWorldScale3D(ProjectileAssetInfo.InitialScale);
	}

	if (ProjectileAssetInfo.HitEffectParticle.IsValid())
	{
		HitNiagaraParticle = ProjectileAssetInfo.HitEffectParticle.Get();
	}
	if (ProjectileAssetInfo.HitEffectParticleLegacy.IsValid())
	{
		HitParticle = ProjectileAssetInfo.HitEffectParticleLegacy.Get();
	}
	if (ProjectileAssetInfo.ExplodeParticle.IsValid())
	{
		ExplodeParticle = ProjectileAssetInfo.ExplodeParticle.Get();
	}
	if (ProjectileAssetInfo.TrailParticle.IsValid())
	{
		TrailParticle->SetAsset(ProjectileAssetInfo.TrailParticle.Get());
	}
	if (ProjectileAssetInfo.HitSound.IsValid())
	{
		HitSound = ProjectileAssetInfo.HitSound.Get();
	}
	if (ProjectileAssetInfo.ExplosionSound.IsValid())
	{
		ExplosionSound = ProjectileAssetInfo.ExplosionSound.Get();
	}
}

void AProjectileBase::DestroyWithEffect(FVector Location, bool bContainCameraShake)
{
	FTransform targetTransform = { FRotator::ZeroRotator, Location, {1.0f, 1.0f, 1.0f} };
	
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Weapon, floor(ProjectileID/10.0)*10, "HitImpact");
	}

	if (HitParticle != nullptr)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, targetTransform);

	if (HitNiagaraParticle != nullptr)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitNiagaraParticle, targetTransform.GetLocation(), targetTransform.GetRotation().Rotator());

	if (HitSound != nullptr)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());

	if (bContainCameraShake)
		GamemodeRef.Get()->PlayCameraShakeEffect(ECameraShakeType::E_Hit, Location, 1.0f);

	Destroy();
	/* 임시 비활성화 251127
	//SetActorLocation(FVector(0.0f, 0.0f, -500.0f));
	ProjectileMovement->Deactivate();

	//Destroy(); deffer 로 변경필요
	FTimerHandle destroyHandle;
	GetWorld()->GetTimerManager().SetTimer(destroyHandle, FTimerDelegate::CreateLambda([this]()
	{
		this->Destroy();
	}), 2.0f, false);*/
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
		tempStream.AddUnique(ProjectileAssetInfo.ExplodeParticle.ToSoftObjectPath());
		tempStream.AddUnique(ProjectileAssetInfo.HitSound.ToSoftObjectPath());
		tempStream.AddUnique(ProjectileAssetInfo.ExplosionSound.ToSoftObjectPath());

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
	//This is Shit. If you add stat, definitely add it to this code.
	ProjectileStat.ProjectileID = NewStat.ProjectileID;
	ProjectileStat.ProjectileName = NewStat.ProjectileName;
	ProjectileStat.Description = NewStat.Description;
	ProjectileStat.bIsExplosive = NewStat.bIsExplosive;
	ProjectileStat.bIsBullet = NewStat.bIsBullet;
	ProjectileStat.ProjectileSpeed = NewStat.ProjectileSpeed;
	ProjectileStat.DamageOverride = NewStat.DamageOverride;
	ProjectileStat.GravityScale = NewStat.GravityScale;
	ProjectileStat.LifeTime = NewStat.LifeTime;
	ProjectileStat.bIsHoming = NewStat.bIsHoming;
	ProjectileStat.HomingAcceleration = NewStat.HomingAcceleration;
	ProjectileStat.TargetingCollisionScale = NewStat.TargetingCollisionScale;
	ProjectileStat.DebuffInfoOverride = NewStat.DebuffInfoOverride;

	// ProjectileStat Debuff info override
	if (ProjectileStat.DebuffInfoOverride)
	{
		ProjectileStat.DebuffInfo = NewStat.DebuffInfo;
	}

	TargetingCollision->SetWorldScale3D(FVector(ProjectileStat.TargetingCollisionScale));
	InitialLifeSpan = ProjectileStat.LifeTime;
	SetLifeSpan(ProjectileStat.LifeTime);
	ProjectileMovement->ProjectileGravityScale =  ProjectileStat.bIsBullet ? 0.0f : 1.0f;
	ProjectileMovement->InitialSpeed = ProjectileStat.ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileStat.ProjectileSpeed;
	ProjectileMovement->Velocity = GetActorRotation().Vector().GetSafeNormal() * ProjectileStat.ProjectileSpeed;
}

void AProjectileBase::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
	, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!ProjectileMovement->IsActive()) return;
	if (!GamemodeRef.IsValid()) return;
	if (!OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AProjectileBase::Explode - OwnerCharacterRef is not valid"));
		return;
	}
	if (!IsValid(OtherActor) || OtherActor->ActorHasTag("Item")) return;
	if (!ProjectileState.bCanAttackCauser && OtherActor == OwnerCharacterRef.Get()) return;
	if (OwnerCharacterRef.Get()->ActorHasTag("Enemy") && OtherActor->ActorHasTag("Enemy")) return;

	if (OtherActor->ActorHasTag("Character"))
	{
		Cast<ACharacterBase>(OtherActor)->ApplyDebuffStack(ProjectileStat.DebuffInfo, OwnerCharacterRef.Get());

		//폭발하는 발사체라면?
		if (ProjectileStat.bIsExplosive)
		{
			Explode(OtherActor);
			return;
		}
		else
		{
			bool bIsFatalAttack = false;
			const float totalDamage = OwnerCharacterRef->WeaponComponent->CalculateTotalDamage(Cast<ACharacterBase>(OtherActor)->GetCharacterGameplayInfo(), bIsFatalAttack);
			UGameplayStatics::ApplyDamage(OtherActor, totalDamage, SpawnInstigator, this, nullptr);				
			DestroyWithEffect(GetActorLocation(), true);
		}
	}
}

void AProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponet, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Hit.bBlockingHit || Hit.GetActor() == OwnerCharacterRef.Get()) return;
	
	FString name = UKismetSystemLibrary::GetDisplayName(OtherComp);
	
	//발사체가 총알일경우, 즉시 제거 
	if (ProjectileStat.bIsBullet)
	{
		if (ProjectileStat.bIsExplosive) Explode(OtherActor);
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
	if (ProjectileStat.bIsExplosive)
	{
		FTimerDelegate timerDelegate;
		timerDelegate.BindUFunction(this, FName("Explode"), OtherActor);
		GetWorldTimerManager().SetTimer(AutoExplodeTimer, timerDelegate, 1.5f, false);
	}
}

void AProjectileBase::CheckInitialDamageCondition(float SphereRadius, float Distance, bool bIsCharacterStandard)
{
	if (!OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AProjectileBase::CheckInitialDamageCondition - OwnerCharacterRef is not valid"));
		return;
	}
	FVector sphereStartLocation;
	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	TArray<AActor*> actorsToIgnore;
	TArray<AActor*> sphereOverlapActors;
	actorsToIgnore.Add(OwnerCharacterRef.Get());

	if (bIsCharacterStandard)
	{
		sphereStartLocation = OwnerCharacterRef.Get()->GetActorLocation();
	}
	else sphereStartLocation = OwnerCharacterRef->WeaponComponent->GetComponentLocation();
	
	sphereStartLocation += OwnerCharacterRef.Get()->GetActorForwardVector() * Distance;;
	objectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	bool result = UKismetSystemLibrary::SphereOverlapActors(GetWorld(), sphereStartLocation, SphereRadius, objectTypes, nullptr, actorsToIgnore, sphereOverlapActors);
	if (result)
	{
		for (AActor* overlapActor : sphereOverlapActors)
		{

			if (!IsValid(overlapActor)) continue;

			if (overlapActor->ActorHasTag("Player"))
			{
				ACharacterBase* player = Cast<ACharacterBase>(overlapActor);
				if (IsValid(player))
				{
					player->ApplyDebuffStack(ProjectileStat.DebuffInfo, OwnerCharacterRef.Get());
				}

				if (ProjectileStat.bIsExplosive)
				{
					Explode(player);
					return;
				}
				else
				{
					bool bIsFatalAttack = false;
					const float totalDamage = OwnerCharacterRef->WeaponComponent->CalculateTotalDamage(Cast<ACharacterBase>(overlapActor)->GetCharacterGameplayInfo(), bIsFatalAttack);
					UGameplayStatics::ApplyDamage(overlapActor, totalDamage, SpawnInstigator, this, nullptr);

					GetWorld()->GetTimerManager().SetTimer(DestroyWithEffectTimer,
						FTimerDelegate::CreateLambda([&]()
							{
								DestroyWithEffect(OwnerCharacterRef.Get()->GetActorLocation(), true);
								GetWorld()->GetTimerManager().ClearTimer(DestroyWithEffectTimer);
							}), 0.1f, false);
				}
			}
		}
	}
}

void AProjectileBase::OnTargetBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	if (ProjectileStat.ProjectileID == 0) return;
	if (!IsValid(OtherActor) || !OwnerCharacterRef.IsValid()) return;
	if (!OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AProjectileBase::OnTargetBeginOverlap - OwnerCharacterRef is not valid"));
		return;
	}
	if (!OtherActor->ActorHasTag("Character")) return;
	if (OtherActor->ActorHasTag(OwnerCharacterRef.Get()->Tags[1]) || OtherActor == this) return;
	if (OtherActor == OwnerCharacterRef.Get()) return;
	if (!ProjectileStat.bIsHoming) return;

	if (OtherActor->ActorHasTag("Player"))
	{
		ACharacterBase* targetCharacter = Cast<ACharacterBase>(OtherActor);
		USceneComponent* TargetComponent = targetCharacter->GetCapsuleComponent();

		ProjectileMovement->HomingTargetComponent = TargetComponent;
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingAccelerationMagnitude = ProjectileStat.HomingAcceleration;
	}
}

void AProjectileBase::Explode(AActor* IgnoreActor)
{
	if (IsActorBeingDestroyed() || !IsValid(this) || !GamemodeRef.IsValid()) return;
	if (!OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AProjectileBase::Explode - OwnerCharacterRef is not valid"));
		return;
	}
	if (!ProjectileStat.bIsExplosive) return;

	//--- 데미지 관련 --------------
	TArray<AActor*> ignoreActors;
	ignoreActors.Add(OwnerCharacterRef.Get());
	ignoreActors.Add(IgnoreActor);
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), ProjectileStat.DamageOverride, ProjectileStat.DamageOverride * 0.2f
		, GetActorLocation(), 125.0f, 250.0f, 0.5f, nullptr, ignoreActors, OwnerCharacterRef.Get()
		, OwnerCharacterRef.Get()->GetController(), ECC_WorldStatic);

	//--- explode 시 근처 캐릭터 오버랩 후 디버프 적용 --------------
	if (!(ProjectileStat.DebuffInfo.Type == ECharacterDebuffType::E_None))
	{
		TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
		TArray<AActor*> outActors;
		traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	}

	//--- 이펙트 출력 --------------
	if (ExplodeParticle != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplodeParticle, GetActorLocation(), GetActorRotation());
	}
	if (ExplosionSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}

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
		TrailParticle->ResetSystem();
		TrailParticle->bAutoActivate = true;
	}
	bIsActivated = true; 
}
