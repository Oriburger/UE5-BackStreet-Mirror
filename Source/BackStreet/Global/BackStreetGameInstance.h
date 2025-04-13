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

//======== SaveSlot Manager=========================
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Save")
		TSubclassOf<class ASaveSlotManager> SaveSlotManagerClassRef;
	
	UFUNCTION()
		bool TrySpawnAndInitializeSaveSlotManager();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ASaveSlotManager* GetSafeSaveSlotManager();

private:
	class ASaveSlotManager* SaveSlotManagerRef;

//======== Handling SaveSlot =========================
protected:
	//UFUNCTION()
		//void OnSlotCreated(const FString& NewSlotName);

	//UFUNCTION()
		//void OnSlotLoaded(const FString& SlotName, bool bIsSuccess);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FORCEINLINE FString GetCurrentSaveSlotName() const { return CurrentSaveSlotName; }

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

//======== SaveGame Data =========================
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
		FProgressSaveData ProgressSaveData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
		FAchievementSaveData AchievementSaveData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
		FInventorySaveData InventorySaveData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
		FString CurrentSaveSlotName;
};
