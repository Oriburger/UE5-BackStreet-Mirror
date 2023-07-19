// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/TransitionManager.h"
#include "../public/ChapterManagerBase.h"
#include "../public/ResourceManager.h"
#include "../public/StageData.h"
#include "../public/GateBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "Engine/LevelStreaming.h"

void UTransitionManager::InitTransitionManager()
{
	// Delegate 왜 안되지..????
	/*if (!SpawnRequestDelegate.IsBound())
	{
		SpawnRequestDelegate.AddDynamic(this, &AResourceManager::SpawnStageActor);
	}*/
	ChapterManager = Cast<AChapterManagerBase>(this->GetOuter());
	LoadCompleteDelegate.BindUFunction(this, "CompleteLoad");
	UnloadCompleteDelegate.BindUFunction(this, "SetGate");

}

void UTransitionManager::InitChapter(TArray<class AStageData*> StageRef)
{
	Stages = StageRef;
	HideStage = nullptr;
}

void UTransitionManager::MoveStage(EDirection Dir)
{
	if (!ChapterManager.IsValid()) return;
	AStageData* currentStage = ChapterManager.Get()->GetCurrentStage();
	if (!IsValid(currentStage)) return;

	MoveDirection = Dir;
	HideStageWork(Dir);

	switch (Dir)
	{
	case EDirection::E_UP:
		ChapterManager.Get()->SetCurrentStage(GetStage(currentStage->GetXPos() + 1, currentStage->GetYPos()));
		LoadStage();
		UE_LOG(LogTemp, Log, TEXT("Move to Up"));
		break;
	case EDirection::E_DOWN:
		ChapterManager.Get()->SetCurrentStage(GetStage(currentStage->GetXPos() - 1, currentStage->GetYPos()));
		LoadStage();
		UE_LOG(LogTemp, Log, TEXT("Move to Down"));
		break;
	case EDirection::E_LEFT:
		ChapterManager.Get()->SetCurrentStage(GetStage(currentStage->GetXPos(), currentStage->GetYPos() - 1));
		LoadStage();
		UE_LOG(LogTemp, Log, TEXT("Move to Left"));
		break;
	case EDirection::E_RIGHT:
		ChapterManager.Get()->SetCurrentStage(GetStage(currentStage->GetXPos(), currentStage->GetYPos() + 1));
		LoadStage();
		UE_LOG(LogTemp, Log, TEXT("Move to Right"));
		break;
	case EDirection::E_Start:
		UE_LOG(LogTemp, Log, TEXT("Start Game"));
		ChapterManager.Get()->SetCurrentStage(Stages[0]);
		LoadStage();
		break;
	case EDirection::E_Chapter:
		UE_LOG(LogTemp, Log, TEXT("New Chapter"));
		ChapterManager.Get()->SetCurrentStage(ChapterManager.Get()->GetLobbyStage());
		UnLoadStage();
		ChapterManager.Get()->MoveChapter();
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("Wrong Dir"));
		break;
	}

	ChapterManager.Get()->UpdateMapUI();
}

void UTransitionManager::LoadStage()
{
	if (!ChapterManager.IsValid()) return;
	AStageData* currentStage = ChapterManager.Get()->GetCurrentStage();
	if (!IsValid(currentStage)) return;
	UE_LOG(LogTemp, Log, TEXT("AStageManagerBase::LoadStage"));

	if (currentStage->GetLevelRef() != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Instance is exist, Load Level"));

		currentStage->GetLevelRef()->OnLevelShown.Add(LoadCompleteDelegate);

		currentStage->GetLevelRef()->SetShouldBeLoaded(true);
		currentStage->GetLevelRef()->SetShouldBeVisible(true);


	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Instance is not exist , Create Level Instance"));

		FString name = FString::FromInt(ChapterManager.Get()->GetChapterLV());
		FVector location = currentStage->GetStageLocation();

		name += FString(TEXT("Stage"));
		name += FString::FromInt(currentStage->GetYPos() * MAX_GRID_SIZE + currentStage->GetXPos());

		currentStage->SetLevelRef(UGameplayStatics::GetStreamingLevel(GetWorld(), currentStage->GetLevelToLoad())->CreateInstance(name));
		ULevelStreaming* level = currentStage->GetLevelRef();

		currentStage->GetLevelRef()->LevelTransform.SetLocation(location);
		
		LoadCompleteDelegate.BindUFunction(this, "CompleteLoad");
		level->OnLevelShown.Add(LoadCompleteDelegate);
		currentStage->GetLevelRef()->SetShouldBeLoaded(true);
		currentStage->GetLevelRef()->SetShouldBeVisible(true);

	}

}

void UTransitionManager::UnLoadStage()
{
	UE_LOG(LogTemp, Log, TEXT("AStageManagerBase::UnLoadStage"));

	if (HideStage.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Instance is exist, Now UnLoad Level"));
		HideStage.Get()->GetLevelRef()->OnLevelHidden.Add(UnloadCompleteDelegate);
		HideStage.Get()->GetLevelRef()->SetShouldBeLoaded(false);
		HideStage.Get()->GetLevelRef()->SetShouldBeVisible(false);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Instance is not exist"));
		SetGate();
	}

}

void UTransitionManager::CompleteLoad()
{
	UE_LOG(LogTemp, Log, TEXT("UTransitionManager::CompleteLoad"));
	if (!ChapterManager.IsValid()) return;
	AStageData* currentStage = ChapterManager.Get()->GetCurrentStage();
	if (!IsValid(currentStage)) return;

	if (!currentStage->GetIsVisited())
	{
		SetStage(currentStage);
		currentStage->SetIsVisited(true);
		// 델리게이트로 변경필요
		ChapterManager.Get()->GetResourceManager()->SpawnStageActor(currentStage);
	}
	else
	{
		currentStage->AIOnDelegate.Broadcast();
	}

	UnLoadStage();

}

void UTransitionManager::SetStage(AStageData* Target)
{
	if (!IsValid(Target)) return;

	UE_LOG(LogTemp, Log, TEXT("UTransitionManager::SetStage"));
	SetSpawnPoint(Target);
	SpawnRequestDelegate.Broadcast(Target);
}

void UTransitionManager::SetSpawnPoint(AStageData* Target)
{
	if (!IsValid(Target)) return;
	UE_LOG(LogTemp, Log, TEXT("UTransitionManager::SetSpawnPoint"));

	ULevelStreaming* levelRef = Target->GetLevelRef();
	ULevel* level = levelRef->GetLoadedLevel();

	for (AActor* actor : level->Actors)
	{
		if (actor->ActorHasTag(FName(TEXT("MonsterSpawnPoint"))))
		{
			Target->AddMonsterSpawnPoint(FVector(actor->GetActorLocation()));
		}
		if (actor->ActorHasTag(FName(TEXT("ItemSpawnPoint"))))
		{
			Target->AddItemSpawnPoint(FVector(actor->GetActorLocation()));
		}
		if (actor->ActorHasTag(FName(TEXT("CharacterSpawnPoint"))))
		{
			Target->AddCharacterSpawnPoint(FVector(actor->GetActorLocation()));
		}
		if (actor->ActorHasTag(FName(TEXT("RewardBoxSpawnPoint"))))
		{
			Target->AddRewardBoxSpawnPoint(FVector(actor->GetActorLocation()));
		}
	}
}

void UTransitionManager::SetGate()
{
	UE_LOG(LogTemp, Log, TEXT("UTransitionManager::SetGate"));
	if (!ChapterManager.IsValid()) return;
	TArray<AActor*> gates;
	AStageData* currentStage = ChapterManager.Get()->GetCurrentStage();
	currentStage->EmptyGateList();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGateBase::StaticClass(), gates);
	for (AActor* gate : gates)
	{
		AGateBase* target = Cast<AGateBase>(gate);
		target->InitGate();

	}
	TeleportCharacter();

}

void UTransitionManager::HideStageWork(EDirection Dir)
{	
	if (!ChapterManager.IsValid()) return;
	if (Dir != EDirection::E_Start)
	{
		HideStage = ChapterManager.Get()->GetCurrentStage();
		HideStage.Get()->AIOffDelegate.Broadcast();
	}
	else
		HideStage = nullptr;
}

void UTransitionManager::TeleportCharacter()
{
	if (!ChapterManager.IsValid()) return;
	UE_LOG(LogTemp, Log, TEXT("UTransitionManager::TeleportCharacter"));
	AStageData* currentStage = ChapterManager.Get()->GetCurrentStage();
	AMainCharacterBase* characterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	FVector moveLocation;
	FName telegate;

	switch (MoveDirection)
	{
	case EDirection::E_UP:
		telegate = FName(TEXT("DOWN"));
		break;
	case EDirection::E_DOWN:
		telegate = FName(TEXT("UP"));
		break;
	case EDirection::E_LEFT:
		telegate = FName(TEXT("RIGHT"));
		break;
	case EDirection::E_RIGHT:
		telegate = FName(TEXT("LEFT"));
		break;
	case EDirection::E_Start:
		telegate = FName(TEXT("StartGate"));
		break;
	case EDirection::E_Chapter:
		telegate = FName(TEXT("ChapterGate"));
		break;
	}

	if (telegate == FName(TEXT("ChapterGate")))
	{
		 TArray<AActor*> gates;

		 UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGateBase::StaticClass(), gates);
		 for (AActor* target : gates)
		 {
			 if (target->ActorHasTag(FName("StartGate")))
				 moveLocation = target->GetActorLocation();
		 }
	}
	else if (telegate == FName(TEXT("StartGate")))
	{
		TArray<FVector> characterSpawnPoint = currentStage->GetCharacterSpawnPoint();

		if (characterSpawnPoint.IsValidIndex(0))
		{
			moveLocation = characterSpawnPoint[0];
		}
	}
	else
	{
		for (class AGateBase* gate : currentStage->GetGateList())
		{
			if (gate->ActorHasTag(telegate))
			{
				UE_LOG(LogTemp, Log, TEXT("GateCheck!"));
				moveLocation = gate->GetActorLocation();
				break;
			}

		}

	}
	moveLocation = moveLocation + FVector(0, 0, 10);
	if(IsValid(characterRef)) characterRef->SetActorLocation(moveLocation);
	else { UE_LOG(LogTemp, Warning, TEXT("Whyrano")); }
}

AStageData* UTransitionManager::GetStage(uint8 XPosition, uint8 YPosition)
{
	if (XPosition >= 0 && XPosition < MAX_GRID_SIZE && YPosition >= 0 && YPosition < MAX_GRID_SIZE)
	{
		return (Stages[(YPosition * MAX_GRID_SIZE) + XPosition]);
	}
	else
		return nullptr;
}