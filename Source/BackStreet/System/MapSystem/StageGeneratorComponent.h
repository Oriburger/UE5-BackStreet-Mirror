// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NewChapterManagerBase.h"
#include "Components/ActorComponent.h"
#include "StageGeneratorComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UStageGeneratorComponent : public UActorComponent
{
	GENERATED_BODY()

//======== Basic ===============
public:	
	// Sets default values for this component's properties
	UStageGeneratorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======== Gameplay Function ===============
public:
	//Init Generator component using chapter info struct
	UFUNCTION()
		void InitGenerator(FChapterInfo NewChapterInfo);

	//Generate new stages info in this chapter
	UFUNCTION()
		TArray<FStageInfo> Generate();

public: //Reward
	//Generate list using probability value list
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FItemInfoDataStruct> GetRewardListFromCandidates(EStageCategoryInfo StageType, TArray<int32> ExceptIDList);

	//Read item info from data table
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FItemInfoDataStruct GetRewardItemInfo(int32 ItemID);

private:
	UPROPERTY()
		TMap<int32, FItemInfoDataStruct> ItemInfoCacheMap;

//======== Getter Function ==============
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetStageCount() { return CurrentChapterInfo.StageCount; }

protected:
	UFUNCTION(BlueprintCallable)
		TSoftObjectPtr<UWorld> GetRandomWorld(TArray<TSoftObjectPtr<UWorld>>& WorldList);

	//Return the randomly shuffled list of world for each stage
	UFUNCTION(BlueprintCallable)
		TArray<TSoftObjectPtr<UWorld> > GetShuffledWorldList(EStageCategoryInfo StageCategory);
	
private:
	FChapterInfo CurrentChapterInfo;

	TWeakObjectPtr<class ABackStreetGameModeBase> GamemodeRef;
};
