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
		FString GetCurrentSaveSlotName() { return CurrentSaveSlotName; }

	UFUNCTION(BlueprintCallable)
		void CacheGameData(FProgressSaveData NewProgressData, FAchievementSaveData NewAchievementData, FInventorySaveData NewInventoryData);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetCachedSaveGameData(FProgressSaveData& OutProgressData, FAchievementSaveData& OutAchievementData, FInventorySaveData& OutInventoryData);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetCachedProgressSaveData(FProgressSaveData& OutProgressData) { OutProgressData = ProgressSaveData; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetCachedAchievementSaveData(FAchievementSaveData& OutAchievementData) { OutAchievementData = AchievementSaveData; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetCachedInventorySaveData(FInventorySaveData& OutInventoryData) { OutInventoryData = InventorySaveData; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsRequiredToLoad() { return bIsRequiredToLoad; }

	UFUNCTION(BlueprintCallable)
		void SetIsRequiredToLoad(bool bIsRequired) { bIsRequiredToLoad = bIsRequired; }

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

	//Save 관련 변수
	bool bIsRequiredToLoad = false;
	FString CurrentSaveSlotName;
};
