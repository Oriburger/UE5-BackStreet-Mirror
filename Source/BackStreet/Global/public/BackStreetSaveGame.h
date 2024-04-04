// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BackStreet.h"
#include "GameFramework/SaveGame.h"
#include "BackStreetSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FGameProgressInfo
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		TMap<EChapterLevel, uint8> CraftingLevelMap;
};

USTRUCT(BlueprintType)
struct FSaveData
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FGameProgressInfo GameProgressInfo;

	UPROPERTY(BlueprintReadWrite)
		int32 count = 0;

};

UCLASS()
class BACKSTREET_API UBackStreetSaveGame : public USaveGame
{
	GENERATED_BODY()
	
//------- init ---------------------------------------	
public:
	UBackStreetSaveGame(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
		FSaveData SaveData;
	
};
