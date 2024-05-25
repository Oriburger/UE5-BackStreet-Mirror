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

//======== Property ===============
public:
	//n*n grid, not working yet
	UPROPERTY(EditDefaultsOnly)
		int32 StageCount = 5; 

private:
	FChapterInfo CurrentChapterInfo;
};
