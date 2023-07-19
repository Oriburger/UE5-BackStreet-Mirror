// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/StageGenerator.h"
#include "../public/ChapterManagerBase.h"
#include "../public/ResourceManager.h"
#include "../../StageSystem/public/StageData.h"


TArray<class AStageData*> UStageGenerator::CreateMaze()
{
	Stages.Empty();
	UWorld* world = GetWorld();

	for (int y = 0; y < MAX_GRID_SIZE; y++)
	{
		for (int x = 0; x < MAX_GRID_SIZE; x++)
		{
			FVector spawnLocation = FVector(x * 5000, y * 5000, 0);
			FActorSpawnParameters spawnParams;
			FRotator rotator;

			AStageData* target = NewObject<AStageData>(this->GetOuter());
			InitStageData(spawnLocation, y, x, target);

			Stages.Push(target);
		
		}

	}	
	
	Tracks.Add(Stages[0]);
	RecursiveBacktracking();

	int32 bossIdx = FMath::RandRange(0, Stages.Num() - 1);
	Stages[bossIdx]->SetStageType(EStageCategoryInfo::E_Boss);

	UE_LOG(LogTemp, Log, TEXT("Complete Create Maze"));


	return Stages;

}

void UStageGenerator::InitStageData(FVector Vector, int32 Ypos, int32 Xpos, class AStageData* Target)
{
	Target->SetXPos(Xpos);
	Target->SetYPos(Ypos);
	Target->SetStageLocation(Vector);

	Target->AddGateInfo(false); // UP
	Target->AddGateInfo(false); // DOWN
	Target->AddGateInfo(false); // LEFT
	Target->AddGateInfo(false); // RIGHT

	Target->SetStageType(EStageCategoryInfo::E_Normal);

	Target->SetIsValid(true);
	Target->SetIsVisited(false);
	Target->SetIsClear(false);

	Target->SetLevelToLoad(Cast<AChapterManagerBase>(this->GetOuter())->GetResourceManager()->GetRandomMap());

}

void UStageGenerator::RecursiveBacktracking()
{
	if (Tracks.Num() == 0) return;

	AStageData* currenttile = Tracks[Tracks.Num() - 1];
	AStageData* nexttile = GetRandomNeighbourTile(currenttile);

	if (nexttile->GetIsValid())
	{
		VisitTile(currenttile, nexttile);
	}
	else
	{
		Tracks.Pop();
	}

	RecursiveBacktracking();
}

class AStageData* UStageGenerator::GetRandomNeighbourTile(class AStageData* Tile)
{
	AStageData* upTile = GetTile(Tile->GetXPos() + 1, Tile->GetYPos());
	AStageData* downTile = GetTile(Tile->GetXPos() - 1, Tile->GetYPos());
	AStageData* leftTile = GetTile(Tile->GetXPos(), Tile->GetYPos() - 1);
	AStageData* rightTile = GetTile(Tile->GetXPos(), Tile->GetYPos() + 1);

	TArray<AStageData*> neighbourTiles;
	if (upTile!=nullptr&&upTile->GetIsValid() && !(upTile->GetGateInfoDir(EDirection::E_UP) || upTile->GetGateInfoDir(EDirection::E_DOWN) || upTile->GetGateInfoDir(EDirection::E_LEFT) || upTile->GetGateInfoDir(EDirection::E_RIGHT))) neighbourTiles.Add(upTile);
	if (downTile!=nullptr&&downTile->GetIsValid() && !(downTile->GetGateInfoDir(EDirection::E_UP) || downTile->GetGateInfoDir(EDirection::E_DOWN) || downTile->GetGateInfoDir(EDirection::E_LEFT) || downTile->GetGateInfoDir(EDirection::E_RIGHT))) neighbourTiles.Add(downTile);
	if (leftTile!=nullptr&&leftTile->GetIsValid() && !(leftTile->GetGateInfoDir(EDirection::E_UP) || leftTile->GetGateInfoDir(EDirection::E_DOWN) || leftTile->GetGateInfoDir(EDirection::E_LEFT) || leftTile->GetGateInfoDir(EDirection::E_RIGHT))) neighbourTiles.Add(leftTile);
	if (rightTile!=nullptr&&rightTile->GetIsValid() && !(rightTile->GetGateInfoDir(EDirection::E_UP) || rightTile->GetGateInfoDir(EDirection::E_DOWN) || rightTile->GetGateInfoDir(EDirection::E_LEFT) || rightTile->GetGateInfoDir(EDirection::E_RIGHT))) neighbourTiles.Add(rightTile);

	if (neighbourTiles.Num() == 0)
	{
		AStageData* target = NewObject<AStageData>();
		target->SetIsValid(false);
		return target;
	}

	int ItemType = FMath::RandRange(0, neighbourTiles.Num() - 1);

	return neighbourTiles[ItemType];
}

void UStageGenerator::VisitTile(class AStageData* CurrentTilePara, class AStageData* NextTilePara)
{
	if (CurrentTilePara->GetXPos() < NextTilePara->GetXPos())
	{
		CurrentTilePara->SetGateInfoDir(EDirection::E_UP, true);
		NextTilePara->SetGateInfoDir(EDirection::E_DOWN, true);
	}
	if (CurrentTilePara->GetXPos() > NextTilePara->GetXPos())
	{
		CurrentTilePara->SetGateInfoDir(EDirection::E_DOWN, true);
		NextTilePara->SetGateInfoDir(EDirection::E_UP, true);
	}
	if (CurrentTilePara->GetYPos() < NextTilePara->GetYPos())
	{
		CurrentTilePara->SetGateInfoDir(EDirection::E_RIGHT, true);
		NextTilePara->SetGateInfoDir(EDirection::E_LEFT, true);
	}
	if (CurrentTilePara->GetYPos() > NextTilePara->GetYPos())
	{
		CurrentTilePara->SetGateInfoDir(EDirection::E_LEFT, true);
		NextTilePara->SetGateInfoDir(EDirection::E_RIGHT, true);
	}

	Tracks.Add(NextTilePara);
}

class AStageData* UStageGenerator::GetTile(int32 XPosition, int32 YPosition)
{
	if (XPosition >= 0 && XPosition < MAX_GRID_SIZE && YPosition >= 0 && YPosition < MAX_GRID_SIZE)
	{
		return (Stages[(YPosition * MAX_GRID_SIZE) + XPosition]);
	}
	return nullptr;
	
}

