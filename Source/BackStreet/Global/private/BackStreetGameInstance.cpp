// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/BackStreetGameInstance.h"
#include "../../Character/public/MainCharacterBase.h"

UBackStreetGameInstance::UBackStreetGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Constructor implementation
}

void UBackStreetGameInstance::Init()
{
	Super::Init();
}


void UBackStreetGameInstance::SaveGameData(FSaveData NewSaveData)
{
	UBackStreetSaveGame* saveGame = NewObject<UBackStreetSaveGame>();
	
	if (saveGame == nullptr) return;
	saveGame->SaveData = NewSaveData;

	UGameplayStatics::SaveGameToSlot(saveGame, SaveSlotName, 0);
}

bool UBackStreetGameInstance::LoadGameSaveData(FSaveData& Result)
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UBackStreetSaveGame* saveGame = Cast<UBackStreetSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		if (saveGame != nullptr)
		{
			Result = saveGame->SaveData;
			return true;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Slot is empty"));
	Result = FSaveData();
	return false;
}