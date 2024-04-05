#pragma once

#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "BackStreetSaveGameStruct.generated.h"

USTRUCT(BlueprintType)
struct FPlayerSaveGameData 
{
public:
	GENERATED_USTRUCT_BODY()

	//======= SaveData of Character Default Stat ======================
	UPROPERTY(BlueprintReadWrite)
		FCharacterStatStruct PlayerStat;

	//======= SaveData of Character Skin(Costume) =====================

};

USTRUCT(BlueprintType)
struct FProgressSaveGameData
{
public:
	GENERATED_USTRUCT_BODY()
	
	//======= SaveData of CraftingSystem===========================
	//Crafting levels by chapter
	UPROPERTY(BlueprintReadWrite)
	TMap<EChapterLevel, uint8> CraftingLevelMap = {{EChapterLevel::E_Chapter1,1}, {EChapterLevel::E_Chapter2,1}, 
																		{EChapterLevel::E_Chapter3,1}, {EChapterLevel::E_Chapter4,1}};
};

USTRUCT(BlueprintType)
struct FPlayStaticsSaveGameData
{
public:
	GENERATED_USTRUCT_BODY()
	
	
};

USTRUCT(BlueprintType)
struct FSaveData
{
public:
	GENERATED_USTRUCT_BODY()

	//Game progress information data that needs to be stored
	UPROPERTY(BlueprintReadWrite)
		FProgressSaveGameData ProgressSaveGameData;

	//Player information data that needs to be stored
	UPROPERTY(BlueprintReadWrite)
		FPlayerSaveGameData PlayerSaveGameData;

	//PlayStaticsData that needs to be stored
	UPROPERTY(BlueprintReadWrite)
		FPlayStaticsSaveGameData PlayStaticsSaveGameData;
};

