// Copyright Epic Games, Inc. All Rights Reserved.

#include "BackStreetGameModeBase.h"
#include "../System/AssetSystem/AssetManagerBase.h"
#include "../System/MapSystem/NewChapterManagerBase.h"
#include "../System/MapSystem/StageManagerComponent.h"
#include "../System/SaveSystem/SaveManager.h"
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

	//------ Ref ���  ---------------
	PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	//----- Asset Manager �ʱ�ȭ -------
	AssetManagerBase = NewObject<UAssetManagerBase>(this, UAssetManagerBase::StaticClass(), FName("AssetManagerBase"));
	AssetManagerBase->InitAssetManager(this);

	//----- SaveManager �ʱ�ȭ -------
	SpawnAndInitializeSaveManager();
}

void ABackStreetGameModeBase::InitialzeGame()
{
	//------ Initialize Chapter Manager ------------
	if (!IsValid(ChapterManagerRef))
	{
		ChapterManagerRef = GetWorld()->SpawnActor<ANewChapterManagerBase>(ChapterManagerClass, FTransform());
	}
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

UCommonUserWidget* ABackStreetGameModeBase::GetMainHUDRef()
{
	if (IsValid(MainHUDRef)) return MainHUDRef;
	return MainHUDRef = Cast<UCommonUserWidget>(CreateWidget(GetWorld(), MainHUDClass));
}

void ABackStreetGameModeBase::CreateDefaultHUD()
{
	UCommonUserWidget* combatWidgetRef = GetMainHUDRef();
	if (IsValid(combatWidgetRef))
	{
		if (!combatWidgetRef->IsInViewport())
		{
			combatWidgetRef->AddToViewport();
		}
		if (Cast<UCommonActivatableWidget>(combatWidgetRef))
		{
			//Cast<UCommonActivatableWidget>(combatWidgetRef)->ActivateWidget();
		}
	}
}

void ABackStreetGameModeBase::SpawnAndInitializeSaveManager()
{
	if (SaveManagerRef != nullptr)	return; // �̹� �����ϸ� �������� ����
	if (!SaveManagerClassRef)
	{
		UE_LOG(LogSaveSystem, Error, TEXT("SaveManagerClassRef�� �������� �ʾҽ��ϴ�!"));
		return;
	}

	// ���忡 ����
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SaveManagerRef = World->SpawnActor<ASaveManager>(SaveManagerClassRef, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (SaveManagerRef)
		{
			// �߰� �ʱ�ȭ�� �ʿ��ϴٸ� ���⿡ �ۼ�
		}
	}
}
