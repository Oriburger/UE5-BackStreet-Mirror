#pragma once

#include "Engine/DataTable.h"
#include "SaveDataStruct.generated.h"

//======= SaveGame Data =========================
USTRUCT(BlueprintType)
struct FProgressSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsTutorialDone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsInGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FChapterInfo ChapterInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FStageInfo StageInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FCharacterGameplayInfo CharacterInfo;
};

USTRUCT(BlueprintType)
struct FAchievementSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 KillCount = 0;
};

USTRUCT(BlueprintType)
struct FInventorySaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 GenesiumCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 FusionCellCount = 0;

};