#pragma once

#include "Engine/DataTable.h"
#include "../../../Global/BackStreet.h"
#include "SaveDataStruct.generated.h"

//======= SaveGame Data ===============================
USTRUCT(BlueprintType)
struct FProgressSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsInitialzed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FChapterInfo ChapterInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FStageInfo StageInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FCharacterGameplayInfo CharacterInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FWeaponStatStruct WeaponStatInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FWeaponStateStruct WeaponStateInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FAbilityManagerInfoStruct AbilityManagerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FItemInventoryInfoStruct InventoryInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSkillManagerInfoStruct SkillManagerInfo;
};

USTRUCT(BlueprintType)
struct FAchievementSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsInitialzed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 KillCount = 0;

	// �÷��̾ �ر��� ��ų ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32> UnlockedPlayerSkillIDList;
};

USTRUCT(BlueprintType)
struct FInventorySaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsInitialzed = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 GenesiumCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 FusionCellCount = 0;
	
};

USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsInitialized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsTutorialDone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 SaveCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SaveSlotName = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsInGame = false;
};