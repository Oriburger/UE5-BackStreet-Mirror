// Copyright Epic Games, Inc. All Rights Reserved.

#include "BackStreetGameModeBase.h"
#include "../Widget/BackStreetWidgetBase.h"
#include "../System/AssetSystem/AssetManagerBase.h"
#include "../System/MapSystem/NewChapterManagerBase.h"
#include "../Character/CharacterBase.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "../Item/ItemBase.h"
#include "SlateBasics.h"
#include "Runtime/UMG/Public/UMG.h"
#include "../Item/Weapon/Throw/ProjectileBase.h"

ABackStreetGameModeBase::ABackStreetGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABackStreetGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	//------ Ref ���  ---------------
	PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	//----- Asset Manager �ʱ�ȭ -------
	AssetManagerBase = NewObject<UAssetManagerBase>(this, UAssetManagerBase::StaticClass(), FName("AssetManagerBase"));
	AssetManagerBase->InitAssetManager(this);

	//------ Initialize Chapter Manager ------------
	ChapterManagerRef = GetWorld()->SpawnActor<ANewChapterManagerBase>(ChapterManagerClass, FTransform());
}

void ABackStreetGameModeBase::InitialzeGame()
{

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

void ABackStreetGameModeBase::ToggleMenuWidget()
{
	if (!IsValid(MenuWidgetRef))
	{
		MenuWidgetRef = Cast<UBackStreetWidgetBase>(CreateWidget(GetWorld(), MenuWidgetClass));
	}
	if (!IsValid(CombatWidgetRef))
	{
		CombatWidgetRef = Cast<UBackStreetWidgetBase>(CreateWidget(GetWorld(), CombatWidgetClass));
	}

	if (IsValid(MenuWidgetRef) && IsValid(CombatWidgetRef))
	{
		if (CombatWidgetRef->IsInViewport())
		{
			// Show menu and hide Combat HUD
			CombatWidgetRef->RemoveFromParent();
			CombatWidgetRef = nullptr;
			MenuWidgetRef->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Combat -> Menu "));
		}
		else
		{
			// Hide menu and show Combat HUD
			MenuWidgetRef->RemoveFromParent();
			MenuWidgetRef = nullptr;
			CombatWidgetRef->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Menu -> Combat"));
		}
	}
}

void ABackStreetGameModeBase::CreateDefaultWidgets()
{
	CombatWidgetRef = Cast<UBackStreetWidgetBase>(CreateWidget(GetWorld(), CombatWidgetClass));
	if (IsValid(CombatWidgetRef))
	{
		CombatWidgetRef->AddToViewport();
	}
	MenuWidgetRef = Cast<UBackStreetWidgetBase>(CreateWidget(GetWorld(), MenuWidgetClass));
}
