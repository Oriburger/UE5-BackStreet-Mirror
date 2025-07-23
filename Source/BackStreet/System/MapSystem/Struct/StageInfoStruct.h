#pragma once

#include "Engine/DataTable.h"
#include "StageInfoStruct.generated.h"

UENUM(BlueprintType)
enum class EStageCategoryInfo : uint8
{
	E_None				  	UMETA(DisplayName = "None"),
	E_Entry					UMETA(DisplayName = "Entry"),
	E_Boss					UMETA(DisplayName = "Boss"),
	E_Craft					UMETA(DisplayName = "Craft"),

	E_Easy					UMETA(DisplayName = "Easy"),
	E_Normal				UMETA(DisplayName = "Normal"),
	E_Hard					UMETA(DisplayName = "Hard"),
	E_Expert				UMETA(DisplayName = "Expert"),
	E_Nightmare				UMETA(DisplayName = "Nightmare"),
};

UENUM(BlueprintType)
enum class EWaveCategoryInfo : uint8
{
	E_None				  		UMETA(DisplayName = "None"),
	E_NomalWave					UMETA(DisplayName = "E_NomalWave"),
	E_TimeLimitWave			  	UMETA(DisplayName = "E_TimeLimitWave"),
};

UENUM(BlueprintType)
enum class EChapterLevel : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Chapter1			UMETA(DisplayName = "Chapter1"),
	E_Chapter2			UMETA(DisplayName = "Chapter2"),
	E_Chapter3			UMETA(DisplayName = "Chapter3"),
	E_Chapter4			UMETA(DisplayName = "Chapter4")
};

USTRUCT(BlueprintType)
struct FEnemyCompositionInfo
{
	GENERATED_BODY()

public:
	//DO NOT ASSIGN THE BOSS ID 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TMap<int32, int32> EnemySet; // EnemyID, spawn count
};

USTRUCT(BlueprintType)
struct FEnemyCompositionNameList
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<FName> CompositionNameList;

	bool IsValid() { return CompositionNameList.Num() > 0; }
};

USTRUCT(BlueprintType)
struct FMultipleStageTypeInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<EStageCategoryInfo> StageTypeList;
};

USTRUCT(BlueprintType)
struct FStageRewardCandidateInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		int32 RewardItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		float RewardItemProbability;
};

USTRUCT(BlueprintType)
struct FStageRewardCandidateInfoList
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		TArray<FStageRewardCandidateInfo> RewardCandidateInfoList;
};

USTRUCT(BlueprintType)
struct FLevelSoftObjectPtrList
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
		TArray<TSoftObjectPtr<UWorld>> LevelList;
};

//이동 예정 LJH
USTRUCT(BlueprintType)
struct FStageInfo
{
	GENERATED_BODY()

public:
	//==== Static Proptery ====================

	//Stage type (entry, combat, time attack, boss, miniGame, gatcha etc..)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EStageCategoryInfo StageType = EStageCategoryInfo::E_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UTexture2D* StageIcon;

	//Map location in 2nd array (현재 선형으로 1차원 X값만 사용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 Coordinate = 0;

	//Main level of this stage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TSoftObjectPtr<UWorld> MainLevelAsset;

	//Outer level of this stage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TSoftObjectPtr<UWorld> OuterLevelAsset;

	//Battle stages only use this value!
	//List of enemy's composition to spawn 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FEnemyCompositionNameList EnemyCompositionInfo;

	//It is declared on tarray because of scalability
	//Temporary Code : stage icon will be replaced with this first member icon
		//the non-combat stage (such as craft, minigame) is exception.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FItemInfoDataStruct> RewardInfoList;

	//==== Dynamic Proptery ====================
	//Dyanmically update after level load

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector> EnemySpawnLocationList;

	//Boss Stage only
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector BossSpawnLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FRotator BossSpawnRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector PlayerStartLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FRotator PlayerStartRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FVector> ItemBoxSpawnLocationList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector RewardSpawnLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FTransform> PortalTransformList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<FName> PortalDirectionTagList;

	//Timed-stage only use this value!
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float TimeLimitValue = 0.0f;

	//Is blocked?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsBlocked = false;

	//Is stage visited?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsVisited = false;

	//Is stage finished?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsFinished = false;

	//Is stage cleared
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsClear = false;

	//Is game over
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bIsGameOver = false;

//---Funciton---------------------------------
public:
	void ClearSpawnLocationInfo()
	{
		UE_LOG(LogTemp, Warning, TEXT("FStageInfo::ClearSpawnLocationList() invoked"));

		// Clear the EnemySpawnLocationList
		EnemySpawnLocationList.Empty();

		// Reset Boss Stage variables
		BossSpawnLocation = FVector::ZeroVector;
		BossSpawnRotation = FRotator::ZeroRotator;

		// Reset Player start location and rotation
		PlayerStartLocation = FVector::ZeroVector;
		PlayerStartRotation = FRotator::ZeroRotator;

		// Clear the ItemBoxSpawnLocationList
		ItemBoxSpawnLocationList.Empty();

		// Reset RewardSpawnLocation
		RewardSpawnLocation = FVector::ZeroVector;

		// Clear the PortalTransformList
		PortalTransformList.Empty();

		// Clear the PortalDirectionTagList
		PortalDirectionTagList.Empty();
	}

	inline bool GetIsCombatStage(bool bContainBossStage = true) const
	{
		return (StageType >= EStageCategoryInfo::E_Easy && StageType <= EStageCategoryInfo::E_Nightmare)
			|| (bContainBossStage && (StageType == EStageCategoryInfo::E_Boss))
			|| StageType == EStageCategoryInfo::E_Entry;
	}
};

USTRUCT(BlueprintType)
struct FStageTemplateInfo
{
	GENERATED_BODY()

public:
	//This value's member must be valid. If not, editor will crash by 'check'
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<EStageCategoryInfo> StageComposition;
};

USTRUCT(BlueprintType)
struct FChapterInfo : public FTableRowBase
{
	GENERATED_BODY()

//======= Gameplay ====================
public:
	//Table Key
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 ChapterID = 0;

	//grid size for chapter
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 1, UIMax = 5), Category = "Category")
		int32 StageCount = 9;

	UPROPERTY(BlueprintReadOnly, Category = "Config")
		int32 CurrentStageCoordinate = 0;

	//Possible stage type per stage level
	//This list's length is must be fit to stage's count(gridsize * 2 - 1)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
		TArray<FMultipleStageTypeInfo> StageTypeListForLevelIdx;

	//Time attack's time limit value
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float EliteTimeAtkStageTimeOut = 60.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float NormalTimeAtkStageTimeOut = 50.0f;

	//Enemy composition information of stage type by name
	//ex) "Tiny_Group" "Tiny_Flame_Group" "Elite_Only" ...
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		TMap<FName, FEnemyCompositionInfo> EnemyCompositionMap;

	//Composition name map for stage type
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		TMap<EStageCategoryInfo, FEnemyCompositionNameList> StageEnemyCompositionInfoMap;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		TMap<EStageCategoryInfo, FStageRewardCandidateInfoList> StageRewardCandidateInfoMap;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		TMap<EStageCategoryInfo, int32> MaxItemBoxSpawnCountMap;

	//스킬이 보상으로 나오는 스테이지의 개수
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		int32 MaxSkillRewardCount = 2;

	//확정적으로 스킬 보상이 나오는 스테이지 ID 리스트
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		TArray<int32> FixedSkillRewardStageIDList;

	// 스킬 보상이 랜덤하게 나올수 있는 스테이지 타입 리스트
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		TArray<EStageCategoryInfo> RandomSkillStageTypeList;

	//Stage template list. If this value is set, this overrides the above values(blocked~, stage tyep~)
	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 1, UIMax = 5), Category = "Config")
		TArray<FStageTemplateInfo> StageTemplateList;

	UPROPERTY(BlueprintReadOnly, Category = "Config")
		TArray<FStageInfo> StageInfoList;
		
//======= State  =======================
	UPROPERTY(BlueprintReadOnly, Category = "State")
		bool bIsChapterInitialized = false;

private:
	UPROPERTY()
		FStageInfo BossStageInfo;
	
public:
//======= Widget ======================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
		TMap<EStageCategoryInfo, UTexture2D*> StageIconInfoMap;

	UPROPERTY(BlueprintReadOnly, Category = "Widget")
		TArray<FVector2D> StageIconTransitionValueList;

//======= Asset =======================
public:
	UPROPERTY(EditDefaultsOnly, Category = "Map")
		TSoftObjectPtr<UWorld> TutorialLevel;

	UPROPERTY(EditDefaultsOnly, Category = "Map")
		TMap<EStageCategoryInfo, FLevelSoftObjectPtrList> StageLevelInfoMap;

	//Outer area level asset
	UPROPERTY(EditDefaultsOnly, Category = "Map")
		TArray<TSoftObjectPtr<UWorld>> OuterStageLevelList;

	//Boss character class per chapter
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		TSubclassOf<class AEnemyCharacterBase> BossCharacterClass;

//======= Function =======================
public:
	void Reset()
	{
		StageInfoList.Empty();
		BossStageInfo = FStageInfo();
		StageLevelInfoMap.Empty();
		OuterStageLevelList.Empty();
		StageIconInfoMap.Empty();
		StageIconTransitionValueList.Empty();
		CurrentStageCoordinate = 0;
	}

	bool IsValid() { return StageInfoList.Num() > 0; }

	TArray<TSoftObjectPtr<UWorld>> GetWorldList(EStageCategoryInfo StageType)
	{
		if (!StageLevelInfoMap.Contains(StageType)) return {};
		return StageLevelInfoMap[StageType].LevelList;
	}
	TSoftObjectPtr<UWorld> GetRandomWorld(EStageCategoryInfo StageType)
	{
		TArray<TSoftObjectPtr<UWorld>> list = GetWorldList(StageType);
		if (!list.IsEmpty())
		{
			return list[FMath::RandRange(0, list.Num() - 1)];
		}
		return nullptr;
	}
	FStageInfo GetBossStageInfo() 
	{ 
		if (BossStageInfo.StageType != EStageCategoryInfo::E_Boss)
		{
			for (auto& stageInfo : StageInfoList)
			{
				if (stageInfo.StageType == EStageCategoryInfo::E_Boss)
				{
					BossStageInfo = stageInfo;
					break;
				}
			}
		}
		return BossStageInfo; 
	}
};