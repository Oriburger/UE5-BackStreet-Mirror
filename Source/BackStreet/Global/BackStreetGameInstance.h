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
	//기존 세이브 매니저 제거
	UFUNCTION()
		void OnPreLoadMap(const FString& MapName);

	//세이브 매니저를 스폰하기 위한 이벤트
	UFUNCTION()
		void OnPostLoadMap(UWorld* LoadedWorld);

	UPROPERTY(EditDefaultsOnly, Category = "Save")
		TSubclassOf<class ASaveSlotManager> SaveSlotManagerClassRef;
	
	UFUNCTION()
		bool TrySpawnAndInitializeSaveSlotManager();

private:
	class ASaveSlotManager* SaveSlotManagerRef;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ASaveSlotManager* GetSafeSaveSlotManager();

//======== Handling SaveSlot =========================
public:
	UFUNCTION(BlueprintCallable, Blueprintpure)
		void GetCurrentSaveSlotInfo(FSaveSlotInfo& OutSaveSlotInfo) { OutSaveSlotInfo = SaveSlotInfo; }

	UFUNCTION(BlueprintCallable)
		void SetCurrentSaveSlotInfo(FSaveSlotInfo NewSaveSlotInfo) 
		{
			SaveSlotInfo = NewSaveSlotInfo; 
			UE_LOG(LogSaveSystem, Log, TEXT("UBackStreetGameInstance::SetCurrentSaveSlotInfo - SaveSlotName: %s, bIsInit ? : %d, bIsInGame ? : %d"), *SaveSlotInfo.SaveSlotName, SaveSlotInfo.bIsInitialized, SaveSlotInfo.bIsInGame);
		}

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FORCEINLINE FString GetCurrentSaveSlotName() const { return SaveSlotInfo.SaveSlotName; }

	UFUNCTION(BlueprintCallable)
		void SetCurrentSaveSlotName(FString NewSaveSlotName);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		void GetIsInGameOrNeutralZone(bool& bIsInGame, bool& bIsNeutralZone);

	UFUNCTION(BlueprintCallable)
		void CacheGameData(FSaveSlotInfo NewSaveSlotInfo, FProgressSaveData NewProgressData, FAchievementSaveData NewAchievementData, FInventorySaveData NewInventoryData);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetCachedSaveGameData(FSaveSlotInfo& OutSaveSlotInfo, FProgressSaveData& OutProgressData, FAchievementSaveData& OutAchievementData, FInventorySaveData& OutInventoryData);
	
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
		FSaveSlotInfo SaveSlotInfo;
};
