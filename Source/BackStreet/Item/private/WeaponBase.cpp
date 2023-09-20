// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/WeaponBase.h"
#include "Components/AudioComponent.h"
#include "../../Global/public/DebuffManager.h"
#include "../../Character/public/CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "../../Global/public/BackStreetGameModeBase.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	DefaultSceneRoot->SetupAttachment(RootComponent);
	SetRootComponent(DefaultSceneRoot);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WEAPON_MESH"));
	WeaponMesh->SetupAttachment(DefaultSceneRoot);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}


void AWeaponBase::InitWeapon(int32 NewWeaponID)
{
	//Stat, State 초기화 
	WeaponID = NewWeaponID;
	WeaponStat.WeaponID = WeaponID;

	if (NewWeaponID == 0)
	{
		WeaponStat.WeaponType = ActorHasTag(FName("Melee")) ? EWeaponType::E_Melee
								: (ActorHasTag(FName("Ranged")) ? EWeaponType::E_Shoot : EWeaponType::E_None);

		UE_LOG(LogTemp, Warning, TEXT("Type!!!!!! = %d"), (int)WeaponStat.WeaponType);
	}

	//FWeaponStatStruct newStat = GetWeaponStatInfoWithID(WeaponID);
	//UpdateWeaponStat(newStat);

	//에셋 초기화
	FWeaponAssetInfoStruct newAssetInfo = GetWeaponAssetInfoWithID(WeaponID);
	WeaponAssetInfo = newAssetInfo; 
	if (WeaponID != 0)
	{
		TArray<FSoftObjectPath> tempStream, assetToStream;
		tempStream.AddUnique(WeaponAssetInfo.WeaponMesh.ToSoftObjectPath());
		tempStream.AddUnique(WeaponAssetInfo.DestroyEffectParticle.ToSoftObjectPath());
		tempStream.AddUnique(WeaponAssetInfo.AttackSound.ToSoftObjectPath());
		tempStream.AddUnique(WeaponAssetInfo.AttackFailSound.ToSoftObjectPath());

		//Melee
		tempStream.AddUnique(WeaponAssetInfo.MeleeWeaponAssetInfo.MeleeTrailParticle.ToSoftObjectPath());
		tempStream.AddUnique(WeaponAssetInfo.MeleeWeaponAssetInfo.HitEffectParticle.ToSoftObjectPath());
		tempStream.AddUnique(WeaponAssetInfo.MeleeWeaponAssetInfo.HitImpactSound.ToSoftObjectPath());

		//Ranged
		//tempStream.AddUnique(WeaponAssetInfo.RangedWeaponAssetInfo.ProjectileClass.ToSoftObjectPath());
		tempStream.AddUnique(WeaponAssetInfo.RangedWeaponAssetInfo.ShootEffectParticle.ToSoftObjectPath());

		for (auto& assetPath : tempStream)
		{
			if (!assetPath.IsValid() || assetPath.IsNull()) continue;
			assetToStream.AddUnique(assetPath);
		}
		FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
		streamable.RequestAsyncLoad(assetToStream, FStreamableDelegate::CreateUObject(this, &AWeaponBase::InitWeaponAsset));
	}
	else WeaponMesh->SetStaticMesh(nullptr);
}


void AWeaponBase::InitWeaponAsset()
{
	if (WeaponAssetInfo.WeaponMesh.IsValid())
	{
		WeaponMesh->SetStaticMesh(WeaponAssetInfo.WeaponMesh.Get());
		WeaponMesh->SetRelativeLocation(WeaponAssetInfo.InitialLocation);
		WeaponMesh->SetRelativeRotation(WeaponAssetInfo.InitialRotation);
		WeaponMesh->SetRelativeScale3D(WeaponAssetInfo.InitialScale);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (WeaponAssetInfo.DestroyEffectParticle.IsValid())
		DestroyEffectParticle = WeaponAssetInfo.DestroyEffectParticle.Get();

	if (WeaponAssetInfo.AttackSound.IsValid())
		AttackSound = WeaponAssetInfo.AttackSound.Get();

	if (WeaponAssetInfo.AttackFailSound.IsValid())
		AttackFailSound = WeaponAssetInfo.AttackFailSound.Get();
}

FWeaponStatStruct AWeaponBase::GetWeaponStatInfoWithID(int32 TargetWeaponID)
{
	if (WeaponStatInfoTable != nullptr && TargetWeaponID != 0)
	{
		FWeaponStatStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetWeaponID);

		newInfo = WeaponStatInfoTable->FindRow<FWeaponStatStruct>(FName(rowName), rowName);
		if (newInfo != nullptr) return *newInfo;
	}
	return FWeaponStatStruct();
}

FWeaponAssetInfoStruct AWeaponBase::GetWeaponAssetInfoWithID(int32 TargetWeaponID)
{
	if (WeaponAssetInfoTable != nullptr && TargetWeaponID != 0)
	{
		FWeaponAssetInfoStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetWeaponID);

		newInfo = WeaponAssetInfoTable->FindRow<FWeaponAssetInfoStruct>(FName(rowName), rowName);
		if (newInfo != nullptr) return *newInfo;
	}
	return FWeaponAssetInfoStruct();
}

void AWeaponBase::UpdateWeaponStat(FWeaponStatStruct NewStat)
{
	WeaponStat = NewStat;
	WeaponState.CurrentDurability = WeaponStat.MaxDurability;
}

void AWeaponBase::RevertWeaponInfo(FWeaponStatStruct OldWeaponStat, FWeaponStateStruct OldWeaponState)
{
	if (OldWeaponStat.WeaponName == FName("")) return;
	WeaponStat = OldWeaponStat;
	WeaponState = OldWeaponState;
}

void AWeaponBase::Attack() {}

void AWeaponBase::StopAttack() { }

void AWeaponBase::UpdateComboState()
{
	WeaponState.ComboCount = (WeaponState.ComboCount + 1); 
}

void AWeaponBase::SetResetComboTimer()
{
	GetWorldTimerManager().ClearTimer(ComboTimerHandle);
	const float delayValue = FMath::Clamp((-1.0f * WeaponStat.WeaponAtkSpeedRate) + 2.0f, 0.25f, 10.0f);

	GetWorldTimerManager().SetTimer(ComboTimerHandle, FTimerDelegate::CreateLambda([&]() {
		WeaponState.ComboCount = 0;
	}), delayValue, false);
}

void AWeaponBase::SetOwnerCharacter(ACharacterBase* NewOwnerCharacterRef)
{
	if (!IsValid(NewOwnerCharacterRef)) return;
	OwnerCharacterRef = NewOwnerCharacterRef;
	SetOwner(OwnerCharacterRef.Get());
}

void AWeaponBase::PlayEffectSound(USoundCue* EffectSound)
{
	if (EffectSound == nullptr || !OwnerCharacterRef.IsValid() || !OwnerCharacterRef.Get()->ActorHasTag("Player")) return;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), EffectSound, GetActorLocation());
}

void AWeaponBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(ComboTimerHandle);
}

void AWeaponBase::UpdateDurabilityState()
{
	if (OwnerCharacterRef.Get()->GetCharacterStat().bInfinite || WeaponStat.bInfinite) return;
	if (--WeaponState.CurrentDurability == 0)
	{
		if (IsValid(DestroyEffectParticle))
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyEffectParticle, GetActorLocation(), FRotator()); //파괴 효과
		}
		ClearAllTimerHandle();
		OwnerCharacterRef.Get()->StopAttack();
		OnWeaponBeginDestroy.Broadcast();
	}
}

float AWeaponBase::GetAttackRange() 
{ 
	if(WeaponStat.WeaponType == EWeaponType::E_Melee) return 25.0f;
	return 200.0f;
}

EWeaponType AWeaponBase::GetWeaponType()
{
	if (ActorHasTag("Melee")) return EWeaponType::E_Melee;
	else if (ActorHasTag("Ranged")) return EWeaponType::E_Shoot;
	else if (ActorHasTag("Throw")) return EWeaponType::E_Throw;
	return EWeaponType();
}
