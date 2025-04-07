// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BackStreet.h"
#include "Engine/GameInstance.h"
#include "BackStreetGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UBackStreetGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	
	//======== SaveGame Data =========================

public:
	UFUNCTION(BlueprintCallable)
		void SaveGameData(FProgressSaveData NewProgressData, FAchievementSaveData NewAchievementData, FInventorySaveData NewInventoryData);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetSaveGameData(FProgressSaveData& OutProgressData, FAchievementSaveData& OutAchievementData, FInventorySaveData& OutInventoryData);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetProgressSaveData(FProgressSaveData& OutProgressData) { OutProgressData = ProgressSaveData; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetAchievementSaveData(FAchievementSaveData& OutAchievementData) { OutAchievementData = AchievementSaveData; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetInventorySaveData(FInventorySaveData& OutInventoryData) { OutInventoryData = InventorySaveData; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
		FProgressSaveData ProgressSaveData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
		FAchievementSaveData AchievementSaveData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
		FInventorySaveData InventorySaveData;
	

//------ PROPERTY ---------------------
private:
	// 레벨 전환 이벤트 핸들러
	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMap(UWorld* LoadedWorld);
};
