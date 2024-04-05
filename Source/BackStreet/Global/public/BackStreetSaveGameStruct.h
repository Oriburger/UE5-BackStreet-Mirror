#pragma once

#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Sound/SoundCue.h"
#include "NiagaraSystem.h"
#include "BackStreetSaveGameStruct.generated.h"


//캐릭터의 애니메이션 에셋 경로를 저장하는 데이터 테이블 구조체
USTRUCT(BlueprintType)
struct FPlayerSaveGameData 
{
public:
	GENERATED_USTRUCT_BODY()

	//======= SaveData of Character Default Stat ======================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float DefaultHP = 100.0f;

	//Multipiler Value
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float DefaultAttack = 1.0f;

	//Multipiler Value
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float DefaultDefense = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float DefaultAttackSpeed = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float DefaultMoveSpeed = 400.0f;

	//======= SaveData of Character Skin(Costume) =====================

};

USTRUCT(BlueprintType)
struct FProgressSaveGameData
{
public:
	GENERATED_USTRUCT_BODY()
	
	//======= SaveData of CraftingSystem===========================
	//Crafting levels by chapter
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
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
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FProgressSaveGameData ProgressSaveGameData;

	//Player information data that needs to be stored
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FPlayerSaveGameData PlayerSaveGameData;

	//PlayStaticsData that needs to be stored
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FPlayStaticsSaveGameData PlayStaticsSaveGameData;
};

