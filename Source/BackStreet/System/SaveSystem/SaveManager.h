// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "SaveManager.generated.h"

class UMasterSaveGame;
class UProgressSaveGame;
class UInventorySaveGame;
class UAchievementSaveGame;

UCLASS()
class BACKSTREET_API ASaveManager : public AActor
{
	GENERATED_BODY()

public:
	ASaveManager();

protected:
	virtual void BeginPlay() override;

public:
	// 저장 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveGameData();

	// 로드 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadGameData();

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

private:
	UPROPERTY()
		class UProgressSaveGame* CachedMasterSave;
	
	// 로딩 여부 플래그
	bool bIsLoaded = false;
	/*
	// 내부 SaveGame 객체 캐시
	UPROPERTY()
	UProgressSaveGame* ProgressData;

	UPROPERTY()
	UInventorySaveGame* InventoryData;

	UPROPERTY()
	UAchievementSaveGame* AchievementData;
	*/

	// SaveGame이 존재하는지 확인
	bool DoesSaveExist() const;

	// SaveGame 생성
	void CreateNewSaveGame();
};
