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

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateGameLoadDone OnInitializeDone;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateGameLoadDone OnSaveBegin;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateGameLoadDone OnSaveDone;

	UFUNCTION(BlueprintNativeEvent)
		void OnPreLoadMap(const FString& MapName);

	UFUNCTION(BlueprintNativeEvent)
		void OnPostLoadMap(UWorld* LoadedWorld);

	UFUNCTION()
		void OnChapterCleared();

	//Total Core Count, Total Gear Count 산출을 위함
	UFUNCTION()
		void OnItemAdded(const FItemInfoDataStruct& NewItemInfo, const int32 AddCount);
	
//======= Basic ======================================
public:
	UFUNCTION()
		void Initialize();	

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		void InitializeReference();

	UFUNCTION()
		void DefferedInitializeReference();

	//EasyGameUI 시스템을 이용해 SaveSlotName을 초기화
	UFUNCTION(BlueprintImplementableEvent)
		void InitializeSaveSlotName();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		float DefferedInitializeTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxDefferedInitializeCount = 100;

private:
	int32 DefferedInitializeCount = 0;

	bool bIsReadyToMoveNeutralZone = true;

	UPROPERTY()
		FTimerHandle DefferedInitializeTimerHandle;

//======= SaveGame Data ========================
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SaveGameData();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsInitialized() const; 

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FString GetSaveSlotName() const;

	UFUNCTION()
		void SetTutorialCompletion(bool bCompleted);

protected:
	UFUNCTION()
		void OnLoadFinished(bool bIsSuccess);

	UFUNCTION(BlueprintCallable)
		void SetSaveSlotName(FString NewSaveSlotName);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
		bool bIsRequiredToLoad = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
		bool bIsInMainMenuLevel = false;

//======= Cache SaveGame Data ========================
protected:
	// 인게임 인스턴스들로부터 게임 데이터를 가져옴
	UFUNCTION(BlueprintCallable)
		void FetchGameData();

    // 최초 스폰 시 Game Instance에서 Cached 데이터를 Fetch
    UFUNCTION(BlueprintCallable, Category = "Save System")
        bool TryFetchCachedData();

    // 특정 시점에 Game Instance로 Cache 연산
    UFUNCTION(BlueprintCallable, Category = "Save System")
        void CacheCurrentGameState();

    // Gamemode(ChapterManager, StageManager), PlayerCharacter(WeaponComponent, InventoryComponent)에 데이터 덮어씌우기
    UFUNCTION(BlueprintCallable, Category = "Save System")
        void ApplyCachedData();

//======= Property ======================================
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
		FSaveSlotInfo SaveSlotInfo;

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