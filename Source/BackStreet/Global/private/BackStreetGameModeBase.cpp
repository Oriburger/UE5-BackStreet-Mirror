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

ABackStreetGameModeBase::ABackStreetGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABackStreetGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	//----- Asset Manager 초기화 -------
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

		//------ 델리게이트 바인딩 ---------------
		FinishChapterDelegate.AddDynamic(this, &ABackStreetGameModeBase::FinishChapter);
		UIAnimationDelegate.AddDynamic(this, &ABackStreetGameModeBase::PlayUIAnimation);

		//------ Ref 멤버 초기화  ---------------
		PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		//------ Global Skill Manager 초기화 --------
		SkillManagerBase = NewObject<USkillManagerBase>(this, USkillManagerBase::StaticClass(), FName("SkillManagerBase"));
		SkillManagerBase->InitSkillManagerBase(this);
	}
}

void ABackStreetGameModeBase::PlayCameraShakeEffect(ECameraShakeType EffectType, FVector Location, float Radius)
{
	if (!IsValid(GetWorld()) || !CameraShakeEffectList.IsValidIndex((uint8)EffectType)) return;
	if (!IsValid(PlayerCharacterRef) || PlayerCharacterRef->IsActorBeingDestroyed()) return;

	Location = Location + (PlayerCharacterRef->FollowingCamera->GetComponentLocation() - PlayerCharacterRef->GetActorLocation());
	UGameplayStatics::PlayWorldCameraShake(GetWorld(), CameraShakeEffectList[(uint8)EffectType], Location, Radius * 0.75f, Radius * 1.5f, 0.5f);
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

			//임시코드!!!!!!!!!!!!!!!! 230704 @ljh
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
