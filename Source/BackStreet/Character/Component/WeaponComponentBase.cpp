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


UWeaponComponentBase::UWeaponComponentBase() {}

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
	
	switch (WeaponStat.WeaponType)
	{
	case EWeaponType::E_Melee:
		MeleeCombatManager->Attack();
		break;
	case EWeaponType::E_Shoot:
		RangedCombatManager->Attack();
		break;
	}
	if (IsValid(WeaponTrailParticle))
	{
		WeaponTrailParticle->Activate(true);
	}
}

void UWeaponComponentBase::StopAttack()
{
	if (!IsValid(MeleeCombatManager) || !IsValid(RangedCombatManager)) return;

	MeleeCombatManager->StopAttack();
	RangedCombatManager->StopAttack();
	if (IsValid(WeaponTrailParticle))
	{
		WeaponTrailParticle->Deactivate();
	}
}

void UWeaponComponentBase::InitWeapon(int32 NewWeaponID)
{
	//Stat, State 초기화 
	int32 oldWeaponID = WeaponID;
	WeaponStat.WeaponID = WeaponID = NewWeaponID;

	//기존꺼 저장
	if (oldWeaponID != 0)
	{
		WeaponStatCacheMap.Add(oldWeaponID, WeaponStat);
		WeaponStateCacheMap.Add(oldWeaponID, WeaponState);
		WeaponAssetCacheMap.Add(oldWeaponID, WeaponAssetInfo);
	}
	
	//캐싱 로직
	if (NewWeaponID == 0)
	{
		WeaponStat = FWeaponStatStruct();
		WeaponState = FWeaponStateStruct();
		this->SetStaticMesh(nullptr);
		return;
	}
	else
	{
		WeaponStat = WeaponStatCacheMap.Contains(NewWeaponID)
					? WeaponStatCacheMap[NewWeaponID]
					: GetWeaponStatInfoWithID(WeaponID);
		WeaponState = WeaponStateCacheMap.Contains(NewWeaponID)
					? WeaponState = WeaponStateCacheMap[NewWeaponID]
					: FWeaponStateStruct();
		if (!WeaponStateCacheMap.Contains(NewWeaponID))
		{
			WeaponState.UpgradedStatMap.Add(EWeaponStatType::E_Attack, 0);
			WeaponState.UpgradedStatMap.Add(EWeaponStatType::E_AttackSpeed, 0);
			WeaponState.UpgradedStatMap.Add(EWeaponStatType::E_FinalAttack, 0);
		}
		WeaponState.RangedWeaponState.UpdateAmmoValidation(WeaponStat.RangedWeaponStat.MaxTotalAmmo);
	}

	//에셋까지 있다면 그냥 반환
	if(WeaponAssetCacheMap.Contains(NewWeaponID))
	{
		WeaponAssetInfo = WeaponAssetCacheMap[NewWeaponID];
		SetProjectileInfo(WeaponAssetInfo.RangedWeaponAssetInfo.ProjectileID);
		InitWeaponAsset();
		return;
	}

	//에셋 초기화
	FWeaponAssetInfoStruct newAssetInfo = GetWeaponAssetInfoWithID(WeaponID);
	WeaponAssetInfo = newAssetInfo;

	if (WeaponID != 0)
	{
		//발사체 초기화
		SetProjectileInfo(WeaponAssetInfo.RangedWeaponAssetInfo.ProjectileID);

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
	if (WeaponStat.WeaponType == EWeaponType::E_Melee)
	{
		OnMainWeaponUpdated.Broadcast();
	}
	OnWeaponStateUpdated.Broadcast(WeaponID, WeaponStat.WeaponType, WeaponState);
}

void UWeaponComponentBase::InitWeaponAsset()
{
	if (!OwnerCharacterRef.IsValid()) return; 

	if (WeaponAssetInfo.WeaponMesh.IsValid())
	{
		this->SetStaticMesh(WeaponAssetInfo.WeaponMesh.Get());
		this->SetRelativeLocation(WeaponAssetInfo.InitialLocation);
		this->SetRelativeRotation(WeaponAssetInfo.InitialRotation);
		this->SetRelativeScale3D(WeaponAssetInfo.InitialScale);
		this->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (IsValid(WeaponTrailParticle))
	{
		WeaponTrailParticle->DestroyComponent();
	}
	if (WeaponAssetInfo.MeleeWeaponAssetInfo.MeleeTrailParticle.IsValid()
		&& !OwnerCharacterRef.Get()->ActorHasTag("Player"))
	{	
		WeaponTrailParticle = NewObject<UNiagaraComponent>(GetOwner());
		WeaponTrailParticle->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
		WeaponTrailParticle->RegisterComponent();
		WeaponTrailParticle->bAutoActivate = false;

		WeaponTrailParticle->SetAsset(WeaponAssetInfo.MeleeWeaponAssetInfo.MeleeTrailParticle.Get());
		FColor weaponTrailParticleColor = WeaponAssetInfo.MeleeWeaponAssetInfo.MeleeTrailParticleColor;
		WeaponTrailParticle->SetColorParameter(FName("Color"), weaponTrailParticleColor);

		FVector startLocation = this->GetSocketLocation("Mid");
		FVector endLocation = this->GetSocketLocation("End");
		FVector socketLocalLocation = UKismetMathLibrary::MakeRelativeTransform(this->GetSocketTransform(FName("End"))
			, this->GetComponentTransform()).GetLocation();

		WeaponTrailParticle->SetVectorParameter(FName("Start"), startLocation);
		WeaponTrailParticle->SetVectorParameter(FName("End"), endLocation);
		WeaponTrailParticle->SetRelativeLocation(socketLocalLocation);
	}

	if (WeaponAssetInfo.MeleeWeaponAssetInfo.HitEffectParticle.IsValid())
	{
		HitEffectParticle = WeaponAssetInfo.MeleeWeaponAssetInfo.HitEffectParticle.Get();
	}
	if (WeaponAssetInfo.MeleeWeaponAssetInfo.HitEffectParticleLarge.IsValid())
	{
		HitEffectParticleLarge = WeaponAssetInfo.MeleeWeaponAssetInfo.HitEffectParticleLarge.Get();
	}
}

FWeaponAssetInfoStruct UWeaponComponentBase::GetWeaponAssetInfoWithID(int32 TargetWeaponID)
{
	if (WeaponAssetCacheMap.Contains(TargetWeaponID)) return WeaponAssetCacheMap[TargetWeaponID];
	if (WeaponAssetInfoTable != nullptr && TargetWeaponID != 0)
	{
		FWeaponAssetInfoStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetWeaponID);

		newInfo = WeaponAssetInfoTable->FindRow<FWeaponAssetInfoStruct>(FName(rowName), rowName);
		WeaponAssetCacheMap.Add(TargetWeaponID, *newInfo);
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


FWeaponStateStruct UWeaponComponentBase::GetWeaponStateCacheDate(int32 TargetWeaponID)
{
	if (!WeaponStateCacheMap.Contains(TargetWeaponID)) return FWeaponStateStruct();
	return WeaponStateCacheMap[TargetWeaponID];
}

bool UWeaponComponentBase::UpdateWeaponStateCache(int32 TargetWeaponID, FWeaponStateStruct NewState)
{
	//@@ 들어오는 정보의 유효성에 대한 검사가 없음 @@ -> 코드 개선 필요
	if (!WeaponStateCacheMap.Contains(TargetWeaponID))
	{
		WeaponStateCacheMap.Add(TargetWeaponID, NewState);
		return true;
	}
	WeaponStateCacheMap[TargetWeaponID] = NewState;
	if (TargetWeaponID == WeaponID) WeaponState = NewState;
	return true;
}

FProjectileStatStruct UWeaponComponentBase::GetProjectileStatInfo(int32 TargetProjectileID)
{
	//캐시에 기록이 되어있다면?
	if (ProjectileStatInfo.ProjectileID == TargetProjectileID) return ProjectileStatInfo;

	//없다면 새로 읽어옴
	else if (ProjectileAssetInfoTable != nullptr && TargetProjectileID != 0)
	{
		FProjectileStatStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetProjectileID);

		newInfo = ProjectileStatInfoTable->FindRow<FProjectileStatStruct>(FName(rowName), rowName);
		if (newInfo != nullptr) return *newInfo;
	}
	return FProjectileStatStruct();
}

FProjectileAssetInfoStruct UWeaponComponentBase::GetProjectileAssetInfo(int32 TargetProjectileID)
{
	//캐시에 기록이 되어있다면?
	if (ProjectileAssetInfo.ProjectileID == TargetProjectileID) return ProjectileAssetInfo;

	//없다면 새로 읽어옴
	else if (ProjectileAssetInfoTable != nullptr && TargetProjectileID != 0)
	{
		FProjectileAssetInfoStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetProjectileID);

		newInfo = ProjectileAssetInfoTable->FindRow<FProjectileAssetInfoStruct>(FName(rowName), rowName);
		if (newInfo != nullptr) return *newInfo;
	}
	return FProjectileAssetInfoStruct();
}

void UWeaponComponentBase::SetProjectileInfo(int32 ProjectileID)
{
	if (WeaponStat.WeaponType != EWeaponType::E_Shoot) return;
	ProjectileStatInfo = GetProjectileStatInfo(WeaponAssetInfo.RangedWeaponAssetInfo.ProjectileID);
	ProjectileAssetInfo = GetProjectileAssetInfo(WeaponAssetInfo.RangedWeaponAssetInfo.ProjectileID);
}

uint8 UWeaponComponentBase::GetLimitedStatLevel(EWeaponStatType WeaponStatType)
{
	checkf(WeaponStat.UpgradableStatInfoMap.Contains(WeaponStatType), TEXT("WeaponType is not valid"));
	for (uint8 level = 0; level < WeaponStat.UpgradableStatInfoMap[WeaponStatType].StatInfoByLevel.Num(); level++)
	{
		if(!WeaponStat.UpgradableStatInfoMap[WeaponStatType].StatInfoByLevel[level].bCanUpgradeLevel) return level-1;
	}
	return WeaponStat.UpgradableStatInfoMap[WeaponStatType].StatInfoByLevel.Num()-1;
}

uint8 UWeaponComponentBase::GetMaxStatLevel(EWeaponStatType WeaponStatType)
{
	checkf(WeaponStat.UpgradableStatInfoMap.Contains(WeaponStatType), TEXT("WeaponType is not valid"));
	return WeaponStat.UpgradableStatInfoMap[WeaponStatType].StatInfoByLevel.Num() - 1;
}

float UWeaponComponentBase::CalculateTotalDamage(FCharacterStateStruct TargetState)
{
	if (!OwnerCharacterRef.IsValid()) return 0.0f;
	FCharacterStateStruct ownerState = OwnerCharacterRef.Get()->GetCharacterState();
	return WeaponStat.WeaponDamage * (1 + FMath::Max(-1, ownerState.TotalAttack - TargetState.TotalDefense))
		* (1 + WeaponStat.bCriticalApply * WeaponStat.CriticalDamageRate)
		+ (!WeaponStat.bFixDamageApply ? 0.0f : WeaponStat.FixedDamageAmount);
}

bool UWeaponComponentBase::UpgradeStat(TArray<uint8> NewLevelList)
{
	for (uint8 idx = 0; idx < MAX_WEAPON_UPGRADABLE_STAT_IDX; idx++)
	{
		EWeaponStatType weaponStatType = StaticCast<EWeaponStatType>(idx+1);
		WeaponState.UpgradedStatMap.Add(weaponStatType, NewLevelList[idx]);
	}
	WeaponStat.WeaponDamage = WeaponStat.UpgradableStatInfoMap[EWeaponStatType::E_Attack].StatInfoByLevel[NewLevelList[0]].StatAdder;
	WeaponStat.WeaponAtkSpeedRate = WeaponStat.UpgradableStatInfoMap[EWeaponStatType::E_AttackSpeed].StatInfoByLevel[NewLevelList[1]].StatAdder;
	WeaponStat.FinalImpactStrength = WeaponStat.UpgradableStatInfoMap[EWeaponStatType::E_FinalAttack].StatInfoByLevel[NewLevelList[2]].StatAdder;
	return true;
}

void UWeaponComponentBase::UpdateComboState()
{
	WeaponState.ComboCount = (WeaponState.ComboCount + 1);
	//OwnerCharacterRef.Get()->GetWeaponInventoryRef()->SyncCurrentWeaponInfo(true);
}

bool UWeaponComponentBase::GetIsFinalCombo()
{
	if (!OwnerCharacterRef.IsValid() || OwnerCharacterRef.Get()->GetMaxComboCount() == 0) return false;
	return GetCurrentComboCnt() % OwnerCharacterRef.Get()->GetMaxComboCount() == 0;
}