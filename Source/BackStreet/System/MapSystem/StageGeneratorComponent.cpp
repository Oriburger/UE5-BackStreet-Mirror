// Fill out your copyright notice in the Description page of Project Settings.


#include "StageGeneratorComponent.h"
#include "NewChapterManagerBase.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UStageGeneratorComponent::UStageGeneratorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UStageGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStageGeneratorComponent::InitGenerator(FChapterInfo NewChapterInfo)
{
	CurrentChapterInfo = NewChapterInfo;
}

TArray<FStageInfo> UStageGeneratorComponent::Generate()
{
	if (CurrentChapterInfo.ChapterID == 0) return TArray<FStageInfo>();

	UE_LOG(LogTemp, Warning, TEXT("UStageGeneratorComponent::Generate() -------"));

	//####### 선형 임시코드 ####### 
	TArray<FStageInfo> result;
	
	FStageInfo temp;
	temp.OuterLevelAsset = CurrentChapterInfo.OuterStageLevelList[0];

	int32 templateIdx = -1;
	FStageTemplateInfo stageTemplateInfo;
	if (CurrentChapterInfo.StageTemplateList.Num() > 0)
	{
		templateIdx = UKismetMathLibrary::RandomInteger(CurrentChapterInfo.StageTemplateList.Num());
		stageTemplateInfo = CurrentChapterInfo.StageTemplateList[templateIdx];
		checkf(stageTemplateInfo.StageComposition.Num() == FMath::Pow(CurrentChapterInfo.GridSize, 2.0f), TEXT("UStageGeneratorComponent::Generate(), invalid template data"));
	}

	UE_LOG(LogTemp, Warning, TEXT("UStageGeneratorComponent::Generate(), template idx : %d"), templateIdx);

	for (int32 stageIdx = 0; stageIdx < FMath::Pow(CurrentChapterInfo.GridSize, 2.0f); stageIdx++)
	{
		//set stage type
		FVector2D stageCoordinate = GetStageCoordinate(stageIdx);
		UE_LOG(LogTemp, Warning, TEXT("Stage Idx : %d,  stageCoordinate : %s"), stageIdx, *stageCoordinate.ToString());
		
		if (templateIdx != -1)
		{
			temp.StageType = stageTemplateInfo.StageComposition[stageIdx];
		}
		else
		{
			TArray<EStageCategoryInfo> stageTypeList = CurrentChapterInfo.StageTypeListForLevelIdx[stageCoordinate.Y + stageCoordinate.X].StageTypeList;
			checkf(stageTypeList.Num() > 0, TEXT("Stage type list data is invalid "));
			temp.StageType = stageTypeList[UKismetMathLibrary::RandomInteger(stageTypeList.Num())];
		}
			
		//set stage coordinate and level
		temp.Coordinate = GetStageCoordinate(stageIdx);
		switch (temp.StageType)
		{
		case EStageCategoryInfo::E_Boss:
			temp.MainLevelAsset = GetRandomWorld(CurrentChapterInfo.BossStageLevelList);
			break;
		case EStageCategoryInfo::E_Combat:
		case EStageCategoryInfo::E_EliteCombat:
			temp.MainLevelAsset = GetRandomWorld(CurrentChapterInfo.CombatStageLevelList);
			break;
		case EStageCategoryInfo::E_Craft:
			temp.MainLevelAsset = GetRandomWorld(CurrentChapterInfo.CraftStageLevelList);
			break;
		case EStageCategoryInfo::E_TimeAttack:
			temp.MainLevelAsset = GetRandomWorld(CurrentChapterInfo.TimeAttackStageLevelList);
			temp.TimeLimitValue = CurrentChapterInfo.NormalTimeAtkStageTimeOut;
			break;
		case EStageCategoryInfo::E_EliteTimeAttack:
			temp.MainLevelAsset = GetRandomWorld(CurrentChapterInfo.TimeAttackStageLevelList);
			temp.TimeLimitValue = CurrentChapterInfo.EliteTimeAtkStageTimeOut;
			break;
		case EStageCategoryInfo::E_Entry:
			temp.MainLevelAsset = GetRandomWorld(CurrentChapterInfo.EntryStageLevelList);
			break;
		case EStageCategoryInfo::E_Gatcha:
			temp.MainLevelAsset = GetRandomWorld(CurrentChapterInfo.GatchaStageLevelList);
			break;
		case EStageCategoryInfo::E_MiniGame:
			temp.MainLevelAsset = GetRandomWorld(CurrentChapterInfo.MinigameStageLevelList);
			break;
		}

		if (temp.StageType == EStageCategoryInfo::E_Boss || temp.StageType == EStageCategoryInfo::E_Combat
			|| temp.StageType == EStageCategoryInfo::E_EliteCombat || temp.StageType == EStageCategoryInfo::E_TimeAttack
			|| temp.StageType == EStageCategoryInfo::E_EliteTimeAttack || temp.StageType == EStageCategoryInfo::E_Entry)
		{
			checkf(CurrentChapterInfo.EnemyCompositionInfoMap.Contains(temp.StageType), TEXT("EnemyCompositionInfoMap is empty"));
			temp.EnemyCompositionInfo = *CurrentChapterInfo.EnemyCompositionInfoMap.Find(temp.StageType);
		}
		result.Add(temp);
	}
	for (FVector2D& coordinate : CurrentChapterInfo.BlockedPosList)
	{
		result[GetStageIdx(coordinate)].bIsBlocked = true;
	}

	return CurrentChapterInfo.StageInfoList = result;
}

FVector2D UStageGeneratorComponent::GetNextCoordinate(FVector2D Direction, FVector2D CurrCoordinate)
{
	CurrCoordinate.Y += Direction.Y;
	CurrCoordinate.X += Direction.X;
	
	if (GetIsCoordinateInBoundary(CurrCoordinate))
	{
		return CurrCoordinate;
	}
	return FVector2D(-1);
}

FVector2D UStageGeneratorComponent::GetStageCoordinate(int32 StageIdx)
{
	if (CurrentChapterInfo.GridSize <= 0 || StageIdx < 0) return FVector2D();
	return FVector2D(StageIdx % CurrentChapterInfo.GridSize, StageIdx / CurrentChapterInfo.GridSize);
}

bool UStageGeneratorComponent::GetIsCoordinateInBoundary(FVector2D Coordinate)
{
	if (Coordinate.Y >= 0 && Coordinate.Y < CurrentChapterInfo.GridSize
		&& Coordinate.X >= 0 && Coordinate.X < CurrentChapterInfo.GridSize) return true;
	return false;
}

TSoftObjectPtr<UWorld> UStageGeneratorComponent::GetRandomWorld(TArray<TSoftObjectPtr<UWorld>>& WorldList)
{
	checkf(!WorldList.IsEmpty(), TEXT("WorldList Is Empty"));
	int32 idx = UKismetMathLibrary::RandomInteger(WorldList.Num());
	return WorldList[idx];
}