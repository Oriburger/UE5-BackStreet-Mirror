// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "Engine/LevelScriptActor.h"
#include "ALevelScriptInGame.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateStageClear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateMissionClear);

UCLASS()
class BACKSTREET_API ALevelScriptInGame : public ALevelScriptActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStageClear StageClearDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateStageClear FadeOutDelegate;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateMissionClear MissionClearDelegate;

public:
	ALevelScriptInGame();
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;


public:
	UFUNCTION(BlueprintCallable)
		void StartGame();

	UFUNCTION(BlueprintCallable)
		void SetGameModeRef();

public:
	UFUNCTION(BlueprintCallable)
		bool IsNewGame();

	UFUNCTION(BlueprintCallable)
		USaveData* LoadSaveData();

	UFUNCTION(BlueprintCallable)
		void SaveData();




public:
	UPROPERTY()
		class AChapterManagerBase* ChapterManager;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AChapterManagerBase* GetChapterManager() { return ChapterManager; }

public:
	UFUNCTION()
		void CreateAssetManager();

	UFUNCTION(BlueprintCallable)
		class AAssetManagerBase* GetAssetManager() { return AssetManager; }

public:
	UFUNCTION(BlueprintImplementableEvent)
		void FadeOut();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateMissionUI(class UMissionBase* target);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateMiniMapUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetMiniMapUI();

	UFUNCTION(BlueprintImplementableEvent)
		void PopUpClearUI();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class AAssetManagerBase> AssetManagerBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class AAssetManagerBase* AssetManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class ABackStreetGameModeBase* GameModeRef;

};
