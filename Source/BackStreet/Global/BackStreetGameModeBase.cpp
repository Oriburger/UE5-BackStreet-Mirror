// Copyright Epic Games, Inc. All Rights Reserved.

#include "BackStreetGameModeBase.h"
#include "BackStreetGameInstance.h"
#include "../System/AssetSystem/AssetManagerBase.h"
#include "../System/MapSystem/NewChapterManagerBase.h"
#include "../System/MapSystem/StageManagerComponent.h"
#include "../System/SaveSystem/SaveSlotManager.h"
#include "../Widget/ScriptManager.h"
#include "../Character/CharacterBase.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "../Item/ItemBase.h"
#include "SlateBasics.h"
#include "CommonActivatableWidget.h"
#include "Runtime/UMG/Public/UMG.h"
#include "../Item/Weapon/Throw/ProjectileBase.h"

ABackStreetGameModeBase::ABackStreetGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABackStreetGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	//------ Ref 멤버  ---------------
	PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	//----- Asset Manager 초기화 -------
	AssetManagerBase = NewObject<UAssetManagerBase>(this, UAssetManagerBase::StaticClass(), FName("AssetManagerBase"));
	AssetManagerBase->InitAssetManager(this);

	//----- ChapterManager 초기화 -------
	ChapterManagerRef = GetWorld()->SpawnActor<ANewChapterManagerBase>(ChapterManagerClass, FTransform());

	//----- ScriptManager 초기화 -------
	ScriptManagerRef = NewObject<UScriptManager>(this, UScriptManager::StaticClass());
	if (ScriptManagerRef)
	{
		ScriptManagerRef->InitScriptSystem(ScriptDataTableAsset);
	}
	
	//----- GameInstance 초기화 -------
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}

void ABackStreetGameModeBase::StartGame(int32 ChapterID)
{
	UE_LOG(LogStage, Warning, TEXT("ABackStreetGameModeBase::StartGame(%d)"), ChapterID);

	if (IsValid(ChapterManagerRef))
	{
		ChapterManagerRef->StartChapter(ChapterID);
	}
	else
	{
		UE_LOG(LogStage, Error, TEXT("ABackStreetGameModeBase::StartGame - ChapterManagerRef is not valid"));
	}
}

void ABackStreetGameModeBase::TryContinueGame()
{
	UE_LOG(LogStage, Warning, TEXT("ABackStreetGameModeBase::ContinueGame()"));
	if (IsValid(ChapterManagerRef))
	{
		ChapterManagerRef->ContinueChapter();
		ChapterManagerRef->StageManagerComponent->OnStageLoadDone.AddDynamic(this, &ABackStreetGameModeBase::CreateDefaultHUD);
	}
	else
	{
		UE_LOG(LogStage, Error, TEXT("ABackStreetGameModeBase::ContinueGame - ChapterManagerRef is not valid"));
	}
}

void ABackStreetGameModeBase::FinishGame(bool bGameIsOver)
{
	if (!IsValid(ChapterManagerRef)) return;
	
	//change level to next chapter
	const int32 currentChapterID = ChapterManagerRef->GetCurrentChapterInfo().ChapterID;
	if (currentChapterID != ChapterManagerRef->GetMaxChapterID() && !bGameIsOver)
	{
		ChapterManagerRef->StartChapter(currentChapterID + 1);
	}
	else
	{
		RequestOpenLevel(FName("NeutralZonePersistent"));
	}
}

void ABackStreetGameModeBase::RegisterActorToStageManager(AActor* TargetActor)
{
	if (!IsValid(TargetActor) || !IsValid(ChapterManagerRef)) return;
	if (!ChapterManagerRef->GetCurrentChapterInfo().IsValid()) return;
	ChapterManagerRef->StageManagerComponent->RegisterActor(TargetActor);
}

void ABackStreetGameModeBase::PlayCameraShakeEffect(ECameraShakeType EffectType, FVector Location, float Radius)
{
	if (!IsValid(GetWorld()) || !CameraShakeEffectList.IsValidIndex((uint8)EffectType)) return;
	if (!IsValid(PlayerCharacterRef) || PlayerCharacterRef->IsActorBeingDestroyed()) return;

	Location = PlayerCharacterRef->FollowingCamera->GetComponentLocation();
	UGameplayStatics::PlayWorldCameraShake(GetWorld(), CameraShakeEffectList[(uint8)EffectType], Location, Radius * 0.75f, Radius * 1.5f, 0.01f);
}

void ABackStreetGameModeBase::ActivateSlowHitEffect(float DilationValue, float Length)
{
	if (!IsValid(PlayerCharacterRef) || PlayerCharacterRef->IsActorBeingDestroyed()) return;

	FTimerHandle attackSlowEffectTimerHandle;
	if (SetGlobalTimeDilationWithFlag(DilationValue, false, false))
	{
		GetWorldTimerManager().SetTimer(attackSlowEffectTimerHandle, this, &ABackStreetGameModeBase::DeactivateSlowHitEffect, Length * DilationValue, false);
	}
}

void ABackStreetGameModeBase::DeactivateSlowHitEffect()
{
	SetGlobalTimeDilationWithFlag(1.0f, false, false);
}

AItemBase* ABackStreetGameModeBase::SpawnItemToWorld(int32 ItemID, FVector SpawnLocation)
{
	if (!IsValid(GetWorld())) return nullptr;
	
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	UClass* targetClass = ItemClass;
	if (ItemClassCacheMap.Contains(ItemID))
	{
		targetClass = ItemClassCacheMap[ItemID];
	}
	else
	{
		FString rowName = FString::FromInt(ItemID);
		FItemInfoDataStruct* newInfo = ItemInfoTable->FindRow<FItemInfoDataStruct>(FName(rowName), rowName);

		if (newInfo != nullptr && newInfo->ItemClass != nullptr)
		{
			ItemClassCacheMap.Add(ItemID, newInfo->ItemClass);
			targetClass = newInfo->ItemClass;
		}
	}

	AItemBase* newItem = Cast<AItemBase>(GetWorld()->SpawnActor(targetClass, &SpawnLocation, nullptr, actorSpawnParameters));
	if (IsValid(newItem))
	{
		newItem->InitItem(ItemID);
		return newItem;
	}
	return nullptr;
}

bool ABackStreetGameModeBase::SetGlobalTimeDilationWithFlag(float DilationValue, bool bIsGlobalSlowEffect, bool bForceApply)
{
	if (bIsGlobalSlowEffectActive && !bForceApply) return false;

	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), DilationValue);
	if (bIsGlobalSlowEffect) bIsGlobalSlowEffectActive = (DilationValue < 1.0f);
	
	return true;
}

void ABackStreetGameModeBase::OnSaveManagerLoadDone(bool bIsSuccess)
{
	if (bIsSuccess)
	{
		if (IsValid(ChapterManagerRef))
		{
			ChapterManagerRef->ContinueChapter();
		}
		else
		{
			UE_LOG(LogStage, Error, TEXT("ABackStreetGameModeBase::OnSaveManagerLoadDone - ChapterManagerRef is not valid"));
		}
	}
	else
	{
		UE_LOG(LogStage, Error, TEXT("ABackStreetGameModeBase::OnSaveManagerLoadDone - Load failed"));
	}
}

UUserWidget* ABackStreetGameModeBase::GetMainHUDRef()
{
	if (IsValid(MainHUDRef)) return MainHUDRef;
	return MainHUDRef = Cast<UUserWidget>(CreateWidget(GetWorld(), MainHUDClass));
}

void ABackStreetGameModeBase::CreateDefaultHUD()
{
	UUserWidget* combatWidgetRef = GetMainHUDRef();
	if (IsValid(combatWidgetRef))
	{
		if (!combatWidgetRef->IsInViewport())
		{
			combatWidgetRef->AddToViewport();
		}
	}

	// 만약, StageLoadDone 이벤트가 바인드 되어있으면 StageLoadDone 이벤트를 제거한다.
	// StageLoadDone 이벤트는 ContinueChapter을 위해 바인드 되어있다.
	if (ChapterManagerRef->StageManagerComponent->OnStageLoadDone.IsBound())
	{
		ChapterManagerRef->StageManagerComponent->OnStageLoadDone.RemoveDynamic(this, &ABackStreetGameModeBase::CreateDefaultHUD);
	}
}

void ABackStreetGameModeBase::RequestOpenLevel(FName MapName)
{
	// 맵 변경
	SetGlobalTimeDilationWithFlag(1.0f, true, true);
	UE_LOG(LogStage, Log, TEXT("ABackStreetGameModeBase::RequestOpenLevel - Opening Level: %s"), *MapName.ToString());
	UGameplayStatics::OpenLevel(GetWorld(), FName(MapName.ToString()));
}