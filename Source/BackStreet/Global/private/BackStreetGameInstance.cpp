// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/BackStreetGameInstance.h"
#include "../../Character/public/MainCharacterBase.h"

UBackStreetGameInstance::UBackStreetGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Constructor implementation
}

//최초 실행 -> SaveGame 오브젝트 초기화 (없다면 생성, 있다면 불러오고 바로 Load)
void UBackStreetGameInstance::Init()
{
	Super::Init();

	TryMakeSaveFile();
}

//단한번만 
void UBackStreetGameInstance::TryMakeSaveFile()
{
	if (SaveGame == nullptr)
	{
		SaveGame = Cast<UBackStreetSaveGame>(UGameplayStatics::CreateSaveGameObject(UBackStreetSaveGame::StaticClass()));
		CharacterInstanceData = SaveGame->SaveData.CharacterInstanceData;
	}	
	else
	{
		LoadGameSaveData();
		CharacterInstanceData = SaveGame->SaveData.CharacterInstanceData;
	}
}

void UBackStreetGameInstance::SaveGameData()
{
	SaveGame->SaveData.CharacterInstanceData = CharacterInstanceData;

	UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
	UE_LOG(LogTemp, Log, TEXT("AfterSave : %d"), SaveGame->SaveData.CharacterInstanceData.CharacterDefaultHP);
}

void UBackStreetGameInstance::LoadGameSaveData()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		SaveGame = Cast<UBackStreetSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		UE_LOG(LogTemp, Log, TEXT("AfterLoad : %d"), SaveGame->SaveData.CharacterInstanceData.CharacterDefaultHP);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Slot is empty"));
	}
}

FSaveData UBackStreetGameInstance::GetCurrentSaveData()
{
	return SaveGame == nullptr ? SaveGame->SaveData : FSaveData();
}

