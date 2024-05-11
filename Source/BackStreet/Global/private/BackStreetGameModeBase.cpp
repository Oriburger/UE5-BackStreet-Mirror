// Copyright Epic Games, Inc. All Rights Reserved.

#include "../public/BackStreetGameModeBase.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
#include "../../StageSystem/public/StageData.h"
#include "../../Item/public/ProjectileBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/RangedWeaponBase.h"
#include "../../Item/public/MeleeWeaponBase.h"
#include "../../Item/public/ItemBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../public/AssetManagerBase.h"
#include "../public/SkillManagerBase.h"
#include "../public/CraftingManagerBase.h"

ABackStreetGameModeBase::ABackStreetGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABackStreetGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	//----- Asset Manager �ʱ�ȭ -------
	AssetManagerBase = NewObject<UAssetManagerBase>(this, UAssetManagerBase::StaticClass(), FName("AssetManagerBase"));
	AssetManagerBase->InitAssetManager(this);
}

void ABackStreetGameModeBase::InitializeGame()
{
	if (bIsInGame)
	{
		FActorSpawnParameters spawnParams;
		FRotator rotator;
		FVector spawnLocation = FVector::ZeroVector;

		CreateChapterManager();
		//ChapterManager = GetWorld()->SpawnActor<AChapterManagerBase>(AChapterManagerBase::StaticClass(), spawnLocation, rotator, spawnParams);
		//ChapterManager->SetOwner(this);
		//ChapterManager->CreateChapterManager();

		//------ ��������Ʈ ���ε� ---------------
		FinishChapterDelegate.AddDynamic(this, &ABackStreetGameModeBase::FinishChapter);
		UIAnimationDelegate.AddDynamic(this, &ABackStreetGameModeBase::PlayUIAnimation);

		//------ Ref ��� �ʱ�ȭ  ---------------
		PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		//------ Global Skill Manager �ʱ�ȭ --------
		SkillManagerBase = NewObject<USkillManagerBase>(this, USkillManagerBase::StaticClass(), FName("SkillManagerBase"));
		SkillManagerBase->InitSkillManagerBase(this);

		//------ Global Crafting Manager �ʱ�ȭ --------
		CraftingManagerBase = NewObject<UCraftingManagerBase>(this, UCraftingManagerBase::StaticClass(), FName("CraftingManagerBase"));
		CraftingManagerBase->InitCraftingManager(this);
	}
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
	if (!IsValid(GetWorld())) return nullptr;
	
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ItemClass != nullptr)
	{
		AItemBase* newItem = Cast<AItemBase>(GetWorld()->SpawnActor(ItemClass, &SpawnLocation, nullptr, actorSpawnParameters));

		if (IsValid(newItem))
		{
			newItem->InitItem(ItemID);

			//�ӽ��ڵ�!!!!!!!!!!!!!!!! 230704 @ljh
			if (IsValid(GetChapterManagerRef()) 
				&& GetChapterManagerRef()->GetCurrentStage())
			{
				GetChapterManagerRef()->GetCurrentStage()->AddItemList(newItem);
			}
		}
		return newItem;
	}
	return nullptr;
}

bool ABackStreetGameModeBase::IsLobbyStage()
{
	if (ChapterManager->GetCurrentStage() == ChapterManager->GetLobbyStage()) 
		return true; 
	else return false;
}

void ABackStreetGameModeBase::UpdateCharacterStat(ACharacterBase* TargetCharacter, FCharacterStatStruct NewStat)
{
	if (IsValid(TargetCharacter))
	{
		TargetCharacter->UpdateCharacterStat(NewStat);
	}
}
