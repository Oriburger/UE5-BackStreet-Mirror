// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponBase.h"
#include "WeaponInventoryBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/AssetSystem/AssetManagerBase.h"
#include "../../Character/CharacterBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"


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
	
	static ConstructorHelpers::FObjectFinder<UDataTable> statInfoTableFinder(TEXT("/Game/Weapon/Data/D_WeaponStatTable.D_WeaponStatTable"));
	static ConstructorHelpers::FObjectFinder<UDataTable> assetInfoTableFinder(TEXT("/Game/Weapon/Data/D_WeaponAssetInfo.D_WeaponAssetInfo"));
	checkf(statInfoTableFinder.Succeeded(), TEXT("statInfoTable 탐색에 실패했습니다."));
	checkf(assetInfoTableFinder.Succeeded(), TEXT("assetInfoTable 클래스 탐색에 실패했습니다."));
	WeaponStatInfoTable = statInfoTableFinder.Object;
	WeaponAssetInfoTable = assetInfoTableFinder.Object;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	AssetManagerBaseRef = GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();
}

void AWeaponBase::InitWeapon(int32 NewWeaponID)
{
	//Stat, State 초기화 
	WeaponID = NewWeaponID;
	WeaponStat.WeaponID = WeaponID;

	if (NewWeaponID == 0)
	{
		WeaponStat = FWeaponStatStruct();
		WeaponState = FWeaponStateStruct();
		WeaponAssetInfo = FWeaponAssetInfoStruct();
		WeaponMesh->SetStaticMesh(nullptr);
	
		if (ActorHasTag(FName("Melee")))
			WeaponStat.WeaponType = EWeaponType::E_Melee;

		else if (ActorHasTag(FName("Throw")))
			WeaponStat.WeaponType = EWeaponType::E_Throw;

		else if (ActorHasTag(FName("Shoot"))) 
			WeaponStat.WeaponType = EWeaponType::E_Shoot;

		else
			WeaponStat.WeaponType =  EWeaponType::E_None;

		return;
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
		tempStream.AddUnique(WeaponAssetInfo.MeleeWeaponAssetInfo.HitEffectParticleLarge.ToSoftObjectPath());
		tempStream.AddUnique(WeaponAssetInfo.MeleeWeaponAssetInfo.HitImpactSoundLarge.ToSoftObjectPath());

		//Ranged
		//tempStream.AddUnique(WeaponAssetInfo.RangedWeaponAssetInfo.ProjectileClass.ToSoftObjectPath());
		tempStream.AddUnique(WeaponAssetInfo.RangedWeaponAssetInfo.ShootEffectParticle.ToSoftObjectPath());

		for (auto& assetPath : tempStream)
		{
			if (!assetPath.IsValid() || !assetPath.IsValid()) continue;
			assetToStream.AddUnique(assetPath);
		}
		FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
		streamable.RequestAsyncLoad(assetToStream, FStreamableDelegate::CreateUObject(this, &AWeaponBase::InitWeaponAsset));
	}
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

float AWeaponBase::CalculateTotalDamage(FCharacterStateStruct TargetState)
{
	if (!OwnerCharacterRef.IsValid()) return 0.0f;
	FCharacterStateStruct ownerState = OwnerCharacterRef.Get()->GetCharacterState();
	return WeaponStat.WeaponDamage * (1 + FMath::Max(-1, ownerState.TotalAttack - TargetState.TotalDefense))
			* (1 + WeaponStat.bCriticalApply * WeaponStat.CriticalDamageRate)
			+ (!WeaponStat.bFixDamageApply ? 0.0f : WeaponStat.FixedDamageAmount);
}

void AWeaponBase::UpdateComboState()
{
	WeaponState.ComboCount = (WeaponState.ComboCount + 1); 
	OwnerCharacterRef.Get()->GetWeaponInventoryRef()->SyncCurrentWeaponInfo(true);
}

void AWeaponBase::SetResetComboTimer()
{
	ResetComboCnt();
}

void AWeaponBase::SetOwnerCharacter(ACharacterBase* NewOwnerCharacterRef)
{
	if (!IsValid(NewOwnerCharacterRef)) return;
	OwnerCharacterRef = NewOwnerCharacterRef;
	SetOwner(OwnerCharacterRef.Get());
}

void AWeaponBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(ComboTimerHandle);
	ComboTimerHandle.Invalidate();
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
		OwnerCharacterRef.Get()->DropWeapon();
		return;
	}
	OwnerCharacterRef.Get()->GetWeaponInventoryRef()->SyncCurrentWeaponInfo(true);
}

float AWeaponBase::GetAttackRange() 
{ 
	if(WeaponStat.WeaponType == EWeaponType::E_Melee) return 25.0f;
	return 200.0f;
}

EWeaponType AWeaponBase::GetWeaponType()
{
	if (ActorHasTag("Melee")) return EWeaponType::E_Melee;
	else if (ActorHasTag("Shoot")) return EWeaponType::E_Shoot;
	else if (ActorHasTag("Throw")) return EWeaponType::E_Throw;
	return EWeaponType();
}
