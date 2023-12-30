#pragma once

#include "Engine/DataTable.h"
#include "../../Character/public/EnemyStatInfoStruct.h"
#include "StageInfoStruct.generated.h"

UENUM(BlueprintType)
enum class EStageCategoryInfo : uint8
{
	E_None				  	UMETA(DisplayName = "None"),
	E_Normal				UMETA(DisplayName = "Normal"),
	E_Mission			  	UMETA(DisplayName = "Mission"),
	E_Boss				    UMETA(DisplayName = "Boss"),
	E_Lobby					UMETA(DisplayName = "Lobby"),

};

UENUM(BlueprintType)
enum class EWaveCategoryInfo : uint8
{
	E_None				  	UMETA(DisplayName = "None"),
	E_NomalWave					UMETA(DisplayName = "E_NomalWave"),
	E_TimeLimitWave			  	UMETA(DisplayName = "E_TimeLimitWave"),
};


USTRUCT(BlueprintType)
struct FWaveEnemyStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TMap<int32, int32> EnemyList; // EnemyID,스폰 수

};

USTRUCT(BlueprintType)
struct FStageInfoStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 StageID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 ChapterLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		EStageCategoryInfo StageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TArray<FName> LevelList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint")
		TArray<UBlueprint*> ItemBoxBPList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint")
		TArray<UBlueprint*> RewardBoxBPList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint")
		TArray<UBlueprint*> CraftingBoxBPList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
		int32 MaxSpawnItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
		int32 MinSpawnItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		EWaveCategoryInfo WaveType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 MaxWave; // 총 웨이브 수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
		TArray<int32> WaveComposition;// 각 웨이브 단계에 스폰할 Enemy 목록의 데이터 테이블 ID를 지님, 항상 총 웨이브 수 만큼의 요소가 들어있어야함

	// 웨이브타입이 디펜스인 경우
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		TMap<int32, float> ClearTimeForEachWave; // 웨이브별 버텨야하는 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		float SpawnInterval;

};


USTRUCT(BlueprintType)
struct FStageDataStruct : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
		int32 XPos;

	UPROPERTY()
		int32 YPos;

	UPROPERTY()
		FVector StageLocation;

	UPROPERTY()
		TArray<bool> GateInfo;

	UPROPERTY()
		EStageCategoryInfo StageCategoryType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FStageInfoStruct StageType;

	UPROPERTY()
		FName LevelToLoad;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		int32 CurrentWaveLevel;

	UPROPERTY()
		bool bIsVisited;

	UPROPERTY()
		bool bIsClear;

	UPROPERTY()
		bool bIsValid;

	UPROPERTY()
		int32 MonsterSpawnPointOrderIdx;

	UPROPERTY()
		TMap<int32, int32> ExistEnemyList;

	UPROPERTY()
		TArray<FVector> MonsterSpawnPoints;

	UPROPERTY()
		TArray<FVector> ItemSpawnPoints;

	UPROPERTY()
		TArray<FVector> CharacterSpawnPoint;

	UPROPERTY()
		TArray<FVector> RewardBoxSpawnPoint;

	UPROPERTY()
		TArray<FVector> CraftingBoxSpawnPoint;

	UPROPERTY()
		TArray<FRotator> CraftingBoxSpawnRotatorPoint;

	UPROPERTY()
		TArray<class AEnemyCharacterBase*> MonsterList;

	UPROPERTY()
		TArray<class AItemBase*> ItemList;

	UPROPERTY()
		TArray<class AItemBoxBase*> ItemBoxList;

	UPROPERTY()
		TArray<class AGateBase*> GateList;

	UPROPERTY()
		class ARewardBoxBase* RewardBoxRef;

	UPROPERTY()
		class ACraftBoxBase* CraftingBoxRef;

	UPROPERTY()
		ULevelStreaming* LevelRef;

};