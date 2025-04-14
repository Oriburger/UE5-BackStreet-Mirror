// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "SaveSlotManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateGameLoadDone, bool, bIsSuccess);

//======== SaveGame Class =========================
UCLASS()
class BACKSTREET_API ASaveSlotManager : public AActor
{
	GENERATED_BODY()


public:
	ASaveSlotManager();

//======== Delegate ============
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateGameLoadDone OnLoadDone;

protected:
	UFUNCTION(BlueprintImplementableEvent)
		void OnPreLoadMap(const FString& MapName);

	UFUNCTION(BlueprintImplementableEvent)
		void OnPostLoadMap(UWorld* LoadedWorld);

	
//======= Basic ======================================
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		void InitializeReference(bool bIsInGame = true);

	UFUNCTION()
		void DefferedInitializeReference();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float DefferedInitializeTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxDefferedInitializeCount = 100;

private:
	int32 DefferedInitializeCount = 0;

	UPROPERTY()
		FTimerHandle DefferedInitializeTimerHandle;

//======= SaveGame Data ========================
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SaveGameData();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FORCEINLINE bool GetIsInitialized() const { return bIsInitialized; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FORCEINLINE FString GetSaveSlotName() const;

	UFUNCTION(BlueprintCallable)
		void SetSaveSlotName(FString NewSaveSlotName);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
		bool bIsRequiredToLoad = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
		bool bIsInitialized = false;

//======= Cache SaveGame Data ========================
public:
    // 🔹 최초 스폰 시 Game Instance에서 Cached 데이터를 Fetch
    UFUNCTION(BlueprintCallable, Category = "Save System")
        void FetchCachedData();

    // 🔹 특정 시점에 Game Instance로 Cache 연산
    UFUNCTION(BlueprintCallable, Category = "Save System")
        void CacheCurrentGameState();

    // 🔹 Gamemode(ChapterManager, StageManager), PlayerCharacter(WeaponComponent, InventoryComponent)에 데이터 덮어씌우기
    UFUNCTION(BlueprintCallable, Category = "Save System")
        void ApplyCachedData();

//======= Property ======================================
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
		FProgressSaveData ProgressSaveData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
		FAchievementSaveData AchievementSaveData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
		FInventorySaveData InventorySaveData;

private:
	UPROPERTY()
		TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	UPROPERTY()
		TWeakObjectPtr<class UBackStreetGameInstance> GameInstanceRef;

	UPROPERTY()
		TWeakObjectPtr<class ANewChapterManagerBase> ChapterManagerRef;

	UPROPERTY()
		TWeakObjectPtr<class UStageManagerComponent> StageManagerRef;

	UPROPERTY()
		TWeakObjectPtr<class AMainCharacterBase> PlayerCharacterRef;

	UPROPERTY()
		TWeakObjectPtr<class UItemInventoryComponent> PlayerInventoryRef;

	UPROPERTY()
		TWeakObjectPtr<class UWeaponComponentBase> PlayerWeaponRef;
};
