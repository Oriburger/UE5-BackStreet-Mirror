// Fill out your copyright notice in the Description page of Project Settings.


#include "StageManagerComponent.h"

// Sets default values for this component's properties
UStageManagerComponent::UStageManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UStageManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UStageManagerComponent::InitStage(FStageInfo NewStageInfo)
{
	if (IsValid(MainAreaRef)) ClearPreviousLevelInstance();
	//if(!NewStageInfo->IsValid()) return;

	CurrentStageInfo = NewStageInfo;
	CreateLevelInstance(NewStageInfo.LevelAssetName, NewStageInfo.OuterLevelAssetName);

	GetWorld()->GetTimerManager().SetTimer(LoadCheckTimerHandle, this, &UStageManagerComponent::CheckLoadStatusAndStartGame, 1.0f, true);
}

void UStageManagerComponent::CreateLevelInstance(FName LevelName, FName OuterLevelName)
{
	//Init loading screen


	//Start load
	bool result = false;
	LoadStatus = (OuterLevelName.IsNone() ? 1 : 2);

	MainAreaRef = MainAreaRef->LoadLevelInstance(GetWorld(), LevelName.ToString(), FVector(0.0f)
												, FRotator::ZeroRotator, result);
	OuterAreaRef = OuterAreaRef->LoadLevelInstance(GetWorld(), OuterLevelName.ToString(), FVector(0.0f)
												, FRotator::ZeroRotator, result);

	MainAreaRef->OnLevelLoaded.AddDynamic(this, &UStageManagerComponent::UpdateLoadStatusCount);
	OuterAreaRef->OnLevelLoaded.AddDynamic(this, &UStageManagerComponent::UpdateLoadStatusCount);
}

void UStageManagerComponent::ClearPreviousLevelInstance()
{
	if (!IsValid(MainAreaRef) || !IsValid(OuterAreaRef)) return; 
	MainAreaRef->SetIsRequestingUnloadAndRemoval(true);
	OuterAreaRef->SetIsRequestingUnloadAndRemoval(true);
}

void UStageManagerComponent::UpdateSpawnPointProperty()
{
	if (!IsValid(MainAreaRef)) return;

	TArray<AActor*> spawnPointList;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Point"), spawnPointList);

	for (AActor*& spawnPoint : spawnPointList)
	{
		if (!IsValid(spawnPoint) || !spawnPoint->Tags.IsValidIndex(1)) continue;
		
		if (spawnPoint->Tags[1] == FName("Enemy"))
		{
			CurrentStageInfo.EnemySpawnLocationList.Add(spawnPoint->GetActorLocation());
		}
		else if (spawnPoint->Tags[1] == FName("PlayerStart"))
		{
			CurrentStageInfo.PlayerStartLocation = spawnPoint->GetActorLocation();
		}
		else if (spawnPoint->Tags[1] == FName("Gate"))
		{
			CurrentStageInfo.GateLocationList.Add(spawnPoint->GetActorLocation());
		}
	}
}

void UStageManagerComponent::SpawnEnemy()
{

}

void UStageManagerComponent::CheckLoadStatusAndStartGame()
{
	if (GetWorld()->GetTimerManager().GetTimerElapsed(LoadCheckTimerHandle) > LoadTimeOut) return;

	if (GetLoadIsDone())
	{
		//Clear timer and invalidate
		GetWorld()->GetTimerManager().ClearTimer(LoadCheckTimerHandle);
		LoadCheckTimerHandle.Invalidate();

		//post load events
		UpdateSpawnPointProperty();

		//spawn enemy and start
		SpawnEnemy();
		StartStage();
	}
}

void UStageManagerComponent::StartStage()
{
	//Move player to start location
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	
	if (IsValid(playerCharacter))
	{
		playerCharacter->SetActorLocation(CurrentStageInfo.PlayerStartLocation);
	}
}

void UStageManagerComponent::FinishStage(bool bIsGameOver)
{
	//Spawn gate 
}
