// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Actor.h"
#include "NewChapterManagerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateChapterClear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateChapterInfoUpdate, FChapterInfo, ChapterInfo);

UCLASS()
class BACKSTREET_API ANewChapterManagerBase : public AActor
{
	GENERATED_BODY()

//======== Delegate ============
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateChapterClear OnChapterCleared;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateChapterInfoUpdate OnChapterInfoUpdated;

//======== Basic ===============
public:
	// Sets default values for this actor's properties
	ANewChapterManagerBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UStageManagerComponent* StageManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		class UStageGeneratorComponent* StageGeneratorComponent;

//======== Main Function ===============
public:
	//Start new chapter using id (data table)
	UFUNCTION(BlueprintCallable)
		void StartChapter(int32 NewChapterID);

	//Finish current chapter
	UFUNCTION()
		void FinishChapter(bool bChapterClear);

	//?
	UFUNCTION()
		void ResetChapter();

	//Move next stage
	UFUNCTION(BlueprintCallable)
		void MoveStage(FVector2D direction);

protected:
	//Init chapter using data table 
	//It must be called before "StartChapter"
	UFUNCTION()
		void InitChapter(int32 NewChapterID);

	//Delegate function called by UStageManageComponent
	UFUNCTION()
		void OnStageFinished(FStageInfo StageInfo);

	UFUNCTION(BlueprintCallable)
		void InitStageIconTranslationList(FVector2D Threshold);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FChapterInfo GetCurrentChapterInfo() { return CurrentChapterInfo; }

	UFUNCTION(BlueprintCallable)
		void SetStageIconTranslationList(TArray<FVector2D> NewList) { CurrentChapterInfo.StageIconTransitionValueList = NewList; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FStageInfo& GetStageInfo(int32 StageIdx);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FStageInfo& GetStageInfoWithCoordinate(FVector2D StageCoordinate);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FName GetStageTypeName(EStageCategoryInfo StageType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsStageBlocked(FVector2D StageCoordinate);

	// if you edit this return value, the new result will not be applyed because it is copy value.
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FStageInfo> GetStageInfoList() { return CurrentChapterInfo.StageInfoList; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector2D GetCurrentLocation() { return CurrentChapterInfo.CurrentStageCoordinate;  }

protected:
	//Assign on blueprint
	UPROPERTY(EditDefaultsOnly, Category = "Data")
		UDataTable* ChapterInfoTable;

private:
	int32 ChapterID;

	FChapterInfo CurrentChapterInfo;

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	TWeakObjectPtr<class AMainCharacterBase> PlayerRef;

	bool bIsChapterFinished = false;

//========= Widget =====================
public:
	//Result widget
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<class UUserWidget> ChapterClearWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<class UUserWidget> GameOverWidgetClass;

protected:
	UFUNCTION()
		void CreateGameResultWidget(bool bChapterClear);

private:
	class UUserWidget* GameResultWidgetRef;

//======== Timer ==========================
protected:
	UFUNCTION()
		void OpenMainMenuLevel();

private:
	FTimerHandle OpenLevelDelayHandle; 
};
