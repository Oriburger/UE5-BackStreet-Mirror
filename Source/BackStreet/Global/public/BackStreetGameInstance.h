// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BackStreet.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BackStreetSaveGame.h"
#include "BackStreetGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UBackStreetGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UBackStreetGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init() override;

public:
	UFUNCTION(BlueprintCallable)
		void SaveGameData(FSaveData NewSaveData);

	UFUNCTION(BlueprintCallable)
		bool LoadGameSaveData(FSaveData& Result);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
		FString SaveSlotName = "BackStreetSavedData";
};
