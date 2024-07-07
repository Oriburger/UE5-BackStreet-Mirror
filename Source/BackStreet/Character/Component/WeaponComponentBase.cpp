// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponentBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/AssetSystem/AssetManagerBase.h"
#include "../../Character/CharacterBase.h"
#include "../../Item/Weapon/CombatManager.h"
#include "../../Item/Weapon/Melee/MeleeCombatManager.h"
#include "../../Item/Weapon/Ranged/RangedCombatManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"


UWeaponComponentBase::UWeaponComponentBase() 
{
}

void UWeaponComponentBase::BeginPlay()
{
	Super::BeginPlay();

	//Gamemode and owner character
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());

	//Init combat manager
	MeleeCombatManager = NewObject<UMeleeCombatManager>(this, FName("MELEECOMBAT_MANAGER"));
	MeleeCombatManager->InitCombatManager(GamemodeRef.Get(), OwnerCharacterRef.Get(), this);

	RangedCombatManager = NewObject<URangedCombatManager>(this, FName("RANGEDCOMBAT_MANAGER"));
	RangedCombatManager->InitCombatManager(GamemodeRef.Get(), OwnerCharacterRef.Get(), this);
}

void UWeaponComponentBase::Attack()
{
	if (!IsValid(MeleeCombatManager) || !IsValid(RangedCombatManager)) return;

	UE_LOG(LogTemp, Warning, TEXT("MELLE #0"));

	switch (WeaponStat.WeaponType)
	{
	case EWeaponType::E_Melee:
		UE_LOG(LogTemp, Warning, TEXT("MELLE #0"));
		MeleeCombatManager->Attack();
		break;
	case EWeaponType::E_Shoot:
		UE_LOG(LogTemp, Warning, TEXT("RANGED #0"));
		RangedCombatManager->Attack();
		break;
	}
}

void UWeaponComponentBase::StopAttack()
{
	if (!IsValid(MeleeCombatManager) || !IsValid(RangedCombatManager)) return;

	MeleeCombatManager->StopAttack();
	RangedCombatManager->StopAttack();
}

void UWeaponComponentBase::InitWeapon(int32 NewWeaponID)
{
	//Stat, State 초기화 
	WeaponID = NewWeaponID;
	WeaponStat.WeaponID = WeaponID;

	if (NewWeaponID == 0)
	{
		WeaponStat = FWeaponStatStruct();
		WeaponState = FWeaponStateStruct();
		WeaponAssetInfo = FWeaponAssetInfoStruct();
		this->SetStaticMesh(nullptr);
		return;
	}
	WeaponStat = GetWeaponStatInfoWithID(WeaponID);

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
		streamable.RequestAsyncLoad(assetToStream, FStreamableDelegate::CreateUObject(this, &UWeaponComponentBase::InitWeaponAsset));
	}
}

void UWeaponComponentBase::InitWeaponAsset()
{
	if (WeaponAssetInfo.WeaponMesh.IsValid())
	{
		this->SetStaticMesh(WeaponAssetInfo.WeaponMesh.Get());
		this->SetRelativeLocation(WeaponAssetInfo.InitialLocation);
		this->SetRelativeRotation(WeaponAssetInfo.InitialRotation);
		this->SetRelativeScale3D(WeaponAssetInfo.InitialScale);
		this->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

FWeaponAssetInfoStruct UWeaponComponentBase::GetWeaponAssetInfoWithID(int32 TargetWeaponID)
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

FWeaponStatStruct UWeaponComponentBase::GetWeaponStatInfoWithID(int32 TargetWeaponID)
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

float UWeaponComponentBase::CalculateTotalDamage(FCharacterStateStruct TargetState)
{
	if (!OwnerCharacterRef.IsValid()) return 0.0f;
	FCharacterStateStruct ownerState = OwnerCharacterRef.Get()->GetCharacterState();
	return WeaponStat.WeaponDamage * (1 + FMath::Max(-1, ownerState.TotalAttack - TargetState.TotalDefense))
		* (1 + WeaponStat.bCriticalApply * WeaponStat.CriticalDamageRate)
		+ (!WeaponStat.bFixDamageApply ? 0.0f : WeaponStat.FixedDamageAmount);
}

void UWeaponComponentBase::UpdateComboState()
{
	WeaponState.ComboCount = (WeaponState.ComboCount + 1);
	//OwnerCharacterRef.Get()->GetWeaponInventoryRef()->SyncCurrentWeaponInfo(true);
}

bool UWeaponComponentBase::GetIsFinalCombo()
{
	return GetCurrentComboCnt() % OwnerCharacterRef.Get()->GetMaxComboCount() == 0;
}
