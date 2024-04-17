// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BackStreet.h"
#include "GameFramework/SaveGame.h"
#include "BackStreetSaveGame.generated.h"

UCLASS()
class BACKSTREET_API UBackStreetSaveGame : public USaveGame
{
	GENERATED_BODY()
	
//------- init ---------------------------------------	
public:
	//Constructor
	UBackStreetSaveGame(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
		FSaveData SaveData;
	
};
