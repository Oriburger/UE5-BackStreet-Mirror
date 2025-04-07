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

//======= �ֿ� ������Ƽ================================
private:	
	// �ε� ���� �÷���
	bool bIsLoaded = false;

	// SaveGame�� �����ϴ��� Ȯ��
	bool DoesSaveExist() const;

	// SaveGame ����
	void CreateNewSaveGame();


//======= Ref ======================================
private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
	TWeakObjectPtr<class ANewChapterManagerBase> ChapterManagerRef;
	TWeakObjectPtr<class AMainCharacterBase> PlayerCharacterRef;
};
