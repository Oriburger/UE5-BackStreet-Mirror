// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/ALevelScriptInGame.h"
#include "../public/ChapterManagerBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Global/public/AssetManagerBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../public/MissionBase.h"
#include "Engine/LevelStreaming.h"
#include "../../Global/public/SaveData.h"
#include "Kismet/GameplayStatics.h"

ALevelScriptInGame::ALevelScriptInGame()
{


}


// Called every frame
void ALevelScriptInGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelScriptInGame::BeginPlay()
{
	Super::BeginPlay();
	FadeOutDelegate.AddDynamic(this, &ALevelScriptInGame::FadeOut);


}


void ALevelScriptInGame::StartGame()
{
	
	FActorSpawnParameters spawnParams;
	FRotator rotator;
	FVector spawnLocation = FVector::ZeroVector;

	if (AssetManager == nullptr)
		CreateAssetManager();

	ChapterManager = GetWorld()->SpawnActor<AChapterManagerBase>(AChapterManagerBase::StaticClass(), spawnLocation, rotator, spawnParams);
	ChapterManager->InitChapterManager();
	
	GameModeRef->StartChapter();
}

void ALevelScriptInGame::SetGameModeRef()
{
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}

bool ALevelScriptInGame::IsNewGame()
{
	USaveData* LoadGameInstance = LoadSaveData();

	if (LoadGameInstance)
		return false;
	else return true;
}

USaveData* ALevelScriptInGame::LoadSaveData()
{
	USaveData* LoadGameInstance = Cast<USaveData>(UGameplayStatics::CreateSaveGameObject(USaveData::StaticClass()));

	if (LoadGameInstance)
	{
		LoadGameInstance->SaveSlotName = "MySaveGame";
		LoadGameInstance->SaveIndex = 0;

		UE_LOG(LogTemp, Log, TEXT("GateCheck!"));

		LoadGameInstance = Cast<USaveData>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveSlotName, LoadGameInstance->SaveIndex));
		
		if (LoadGameInstance)
		{
			UE_LOG(LogTemp, Log, TEXT("IsNotNewGame!"));
		}
		else
			UE_LOG(LogTemp, Log, TEXT("IsNewGame!"));
		
		return LoadGameInstance;
	}
	else
		return nullptr;

}


void ALevelScriptInGame::SaveData()
{
	USaveData* SaveGameInstance = Cast<USaveData>(UGameplayStatics::CreateSaveGameObject(USaveData::StaticClass()));

	if (SaveGameInstance)
	{
		/** Save file data **/
		SaveGameInstance->SaveSlotName = "MySaveGame";
		SaveGameInstance->SaveIndex = 0;

		/** Save data **/
		SaveGameInstance->SaveName = "Player0";

		SaveGameInstance->IsNewGame = false;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGameInstance is nullptr"));
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->SaveIndex);
}


void ALevelScriptInGame::CreateAssetManager()
{
	AssetManager = GetWorld()->SpawnActor<AAssetManagerBase>(AssetManagerBP, FVector::ZeroVector, FRotator::ZeroRotator);
}

