// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "SaveManager.generated.h"


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

	// ���� ȣ��
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveGameData();

	// �ε� ȣ��
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadGameData();

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Initialize();

	UFUNCTION(BlueprintCallable)
		void RequestLoadGame();

	// ���� ��ȯ ��, SaveManager�� �����Ͱ� ���ǵǱ� ������ GameInstance�� ���� ������ ���־���Ѵ�.
	UFUNCTION()
		void RestoreGameDataFromCache();

//======= ���� ���̺�/�ε� �Լ� =========================
public: 
	// ���� �и� ���� �Լ�
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveProgress();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveInventory();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SaveAchievement();

	// ���� �и� �ε� �Լ�
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadProgress();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadInventory();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void LoadAchievement();

	// Save Slot �̸�
	UPROPERTY(EditDefaultsOnly, Category = "Save")
		FString SaveSlotName = TEXT("MasterSaveSlot");

	UPROPERTY(EditDefaultsOnly, Category = "Save")
		int32 UserIndex = 0;

protected:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void FetchProgressData();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void FetchAchievementData();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void FetchInventoryData();

//======= �ֿ� ������Ƽ================================
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Save")
		FProgressSaveData ProgressSaveData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Save")
		FAchievementSaveData AchievementSaveData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Save")
		FInventorySaveData InventorySaveData;

//======= Ref ======================================
private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
	TWeakObjectPtr<class ANewChapterManagerBase> ChapterManagerRef;
	TWeakObjectPtr<class UBackStreetGameInstance> GameInstanceRef;
	TWeakObjectPtr<class AMainCharacterBase> PlayerCharacterRef;
};
