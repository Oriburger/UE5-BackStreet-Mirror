// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Actor.h"
#include "NewChapterManagerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateChapterClear);

UCLASS()
class BACKSTREET_API ANewChapterManagerBase : public AActor
{
	GENERATED_BODY()

//======== Delegate ============
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateChapterClear OnChapterCleared;

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

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FChapterInfo GetCurrentChapterInfo() { return CurrentChapterInfo; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FStageInfo GetStageInfo(int32 StageIdx);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FStageInfo GetStageInfoWithCoordinate(FVector2D StageCoordinate);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FName GetStageTypeName(EStageCategoryInfo StageType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsStageBlocked(FVector2D StageCoordinate);

	// if you edit this return value, the new result will not be applyed because it is copy value.
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FStageInfo> GetStageInfoList() { return StageInfoList; }

protected:
	//Assign on blueprint
	UPROPERTY(EditDefaultsOnly, Category = "Data")
		UDataTable* ChapterInfoTable;

private:
	int32 ChapterID;

	FChapterInfo CurrentChapterInfo;

	FVector2D CurrentStageLocation;

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	TWeakObjectPtr<class AMainCharacterBase> PlayerRef;

	TArray<FStageInfo> StageInfoList;

	bool bIsChapterFinished = false;

//========= Widget =====================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		UUserWidget* GetCombatWidgetRef() { return CombatWidgetRef; }

	//Result widget
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UUserWidget> ChapterClearWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UUserWidget> CombatWidgetClass;

protected:
	UFUNCTION()
		void CreateGameResultWidget(bool bChapterClear);

	//Add Combat UI
	UFUNCTION()
		void AddCombatWidget();

private:
	UUserWidget* GameResultWidgetRef;

	//For combat hud widget
	UUserWidget* CombatWidgetRef;

//======== Timer ==========================
protected:
	UFUNCTION()
		void OpenMainMenuLevel();

private:
	FTimerHandle OpenLevelDelayHandle; 
};
