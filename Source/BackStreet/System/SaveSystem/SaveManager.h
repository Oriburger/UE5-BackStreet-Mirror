// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "SaveManager.generated.h"


//======= SaveGame Data =========================
USTRUCT(BlueprintType)
struct FProgressSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsTutorialDone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsInGame;

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
		int32 KillCount;
};	

USTRUCT(BlueprintType)
struct FInventorySaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 GenesiumCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 FusionCellCount;

};


//======== SaveGame Class =========================
UCLASS()
class BACKSTREET_API ASaveManager : public AActor
{
	GENERATED_BODY()


public:
	ASaveManager();
	
//======= Basic ======================================
protected:
	virtual void BeginPlay() override;

	// 저장 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveGameData();

	// 로드 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadGameData();

//======= 메인 세이브/로드 함수 =========================
public: 
	// 내부 분리 저장 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveProgress();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveInventory();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveAchievement();

	// 내부 분리 로드 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadProgress();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadInventory();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadAchievement();

	// Save Slot 이름
	UPROPERTY(EditDefaultsOnly, Category = "Save")
		FString SaveSlotName = TEXT("MasterSaveSlot");

	UPROPERTY(EditDefaultsOnly, Category = "Save")
		int32 UserIndex = 0;

//======= 주요 프로퍼티================================
private:	
	// 로딩 여부 플래그
	bool bIsLoaded = false;

	// SaveGame이 존재하는지 확인
	bool DoesSaveExist() const;

	// SaveGame 생성
	void CreateNewSaveGame();


//======= Ref ======================================
private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
	TWeakObjectPtr<class ANewChapterManagerBase> ChapterManagerRef;
	TWeakObjectPtr<class AMainCharacterBase> PlayerCharacterRef;
};
