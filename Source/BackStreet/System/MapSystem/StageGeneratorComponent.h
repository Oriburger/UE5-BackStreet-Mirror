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

//======== Stage 1st to 2nd ==============
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector2D GetNextCoordinate(FVector2D Direction, FVector2D CurrCoordinate);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetStageCount() { return CurrentChapterInfo.GridSize * 2 - 1; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetStageIdx(FVector2D Location) { return CurrentChapterInfo.GridSize * Location.Y + Location.X; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector2D GetStageCoordinate(int32 StageIdx);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsCoordinateInBoundary(FVector2D Coordinate);

protected:
	UFUNCTION(BlueprintCallable)
		TSoftObjectPtr<UWorld> GetRandomWorld(TArray<TSoftObjectPtr<UWorld>>& WorldList);

	//Return the randomly shuffled list of world for each stage
	UFUNCTION(BlueprintCallable)
		TArray<TSoftObjectPtr<UWorld> > GetShuffledWorldList();
	
private:
	FChapterInfo CurrentChapterInfo;
};
