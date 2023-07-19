// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveData.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API USaveData : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
		FString SaveSlotName;		

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
		int32 SaveIndex;		

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
		FString SaveName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
		bool IsNewGame;
	
};
