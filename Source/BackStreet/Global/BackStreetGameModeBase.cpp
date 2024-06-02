// Copyright Epic Games, Inc. All Rights Reserved.

#include "BackStreetGameModeBase.h"
#include "../System/AssetSystem/AssetManagerBase.h"
#include "../System/SkillSystem/SkillManagerBase.h"
#include "../System/MapSystem/NewChapterManagerBase.h"
#include "../System/CraftingSystem/CraftingManagerBase.h"
#include "../Character/CharacterBase.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "../Item/ItemBase.h"
#include "../Item/Weapon/WeaponBase.h"
#include "../Item/Weapon/Throw/ProjectileBase.h"
#include "../Item/Weapon/Ranged/RangedWeaponBase.h"
#include "../Item/Weapon/Melee/MeleeWeaponBase.h"

ABackStreetGameModeBase::ABackStreetGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABackStreetGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void ABackStreetGameModeBase::InitialzeGame()
{
	//----- Asset Manager ÃÊ±âÈ­ -------
	AssetManagerBase = NewObject<UAssetManagerBase>(this, UAssetManagerBase::StaticClass(), FName("AssetManagerBase"));
	AssetManagerBase->InitAssetManager(this);

	//------ Initialize Chapter Manager ------------
	ChapterManagerRef = GetWorld()->SpawnActor<ANewChapterManagerBase>();

	//------ Ref ¸â¹ö  ---------------
	PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	//------ Initialize Global Skill Manager --------
	SkillManagerBase = NewObject<USkillManagerBase>(this, USkillManagerBase::StaticClass(), FName("SkillManagerBase"));
	SkillManagerBase->InitSkillManagerBase(this);

	//------ Initialize Global Crafting Manager --------
	CraftingManagerBase = NewObject<UCraftingManagerBase>(this, UCraftingManagerBase::StaticClass(), FName("CraftingManagerBase"));
	CraftingManagerBase->InitCraftingManager(this);
}

void ABackStreetGameModeBase::StartGame(int32 ChapterID)
{
	InitialzeGame();

	if (IsValid(ChapterManagerRef))
	{
		ChapterManagerRef->StartChapter(ChapterID);
	}
}

void ABackStreetGameModeBase::FinishGame(bool bGameIsOver)
{
	if (!IsValid(ChapterManagerRef)) return;
	ChapterManagerRef->FinishChapter(!bGameIsOver);
}

void ABackStreetGameModeBase::PlayCameraShakeEffect(ECameraShakeType EffectType, FVector Location, float Radius)
{
	if (!IsValid(GetWorld()) || !CameraShakeEffectList.IsValidIndex((uint8)EffectType)) return;
	if (!IsValid(PlayerCharacterRef) || PlayerCharacterRef->IsActorBeingDestroyed()) return;

	Location = PlayerCharacterRef->FollowingCamera->GetComponentLocation();
	UGameplayStatics::PlayWorldCameraShake(GetWorld(), CameraShakeEffectList[(uint8)EffectType], Location, Radius * 0.75f, Radius * 1.5f, 0.5f);
}

void ABackStreetGameModeBase::ActivateSlowHitEffect(float DilationValue, float Length)
{
	if (!IsValid(PlayerCharacterRef) || PlayerCharacterRef->IsActorBeingDestroyed()) return;

	FTimerHandle attackSlowEffectTimerHandle;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), DilationValue);
	GetWorldTimerManager().SetTimer(attackSlowEffectTimerHandle, this, &ABackStreetGameModeBase::DeactivateSlowHitEffect, Length * DilationValue, false);
}

void ABackStreetGameModeBase::DeactivateSlowHitEffect()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

AItemBase* ABackStreetGameModeBase::SpawnItemToWorld(int32 ItemID, FVector SpawnLocation)
{
	/*
	if (!IsValid(GetWorld())) return nullptr;
	
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ItemClass != nullptr)
	{
		AItemBase* newItem = Cast<AItemBase>(GetWorld()->SpawnActor(ItemClass, &SpawnLocation, nullptr, actorSpawnParameters));

		if (IsValid(newItem))
		{
			newItem->InitItem(ItemID);
		}
		return newItem;
	}
	*/
	return nullptr;
}

void ABackStreetGameModeBase::UpdateCharacterStat(ACharacterBase* TargetCharacter, FCharacterStatStruct NewStat)
{
	if (IsValid(TargetCharacter))
	{
		TargetCharacter->UpdateCharacterStat(NewStat);
	}
}

UUserWidget* ABackStreetGameModeBase::GetCombatWidgetRef()
{
	if (!IsValid(ChapterManagerRef)) return nullptr;
	return ChapterManagerRef->GetCombatWidgetRef();
}
