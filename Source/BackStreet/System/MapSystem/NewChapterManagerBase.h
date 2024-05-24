// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Actor.h"
#include "NewChapterManagerBase.generated.h"


UCLASS()
class BACKSTREET_API ANewChapterManagerBase : public AActor
{
	GENERATED_BODY()

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

private:
	int32 ChapterID;

	FChapterInfo CurrentChapterInfo;

	UDataTable* ChapterInfoTable;

	FVector2D CurrentStageLocation;

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;

	TWeakObjectPtr<class AMainCharacterBase> PlayerRef;

	TArray<FStageInfo> StageInfoList;


//========= Widget =====================
protected:
	UFUNCTION()
		void CreateGameResultWidget(bool bChapterClear);

private:
	//Result widget
	TSubclassOf<UUserWidget> ChapterClearWidgetClass;
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UUserWidget* GameResultWidgetRef;
};
