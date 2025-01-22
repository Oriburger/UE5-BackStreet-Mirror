// Fill out your copyright notice in the Description page of Project Settings.


#include "StageGeneratorComponent.h"
#include "../../Global/BackStreetGameModeBase.h"
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

	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
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
	
	FStageInfo stageInfo;
	stageInfo.OuterLevelAsset = CurrentChapterInfo.OuterStageLevelList[0];

	int32 templateIdx = -1;
	FStageTemplateInfo stageTemplateInfo;
	if (CurrentChapterInfo.StageTemplateList.Num() > 0)
	{
		templateIdx = UKismetMathLibrary::RandomInteger(CurrentChapterInfo.StageTemplateList.Num());
		stageTemplateInfo = CurrentChapterInfo.StageTemplateList[templateIdx];
		checkf(stageTemplateInfo.StageComposition.Num() == FMath::Pow(CurrentChapterInfo.GridSize, 2.0f), TEXT("UStageGeneratorComponent::Generate(), invalid template data"));
	}
	UE_LOG(LogTemp, Warning, TEXT("UStageGeneratorComponent::Generate(), template idx : %d"), templateIdx);

	TArray<TSoftObjectPtr<UWorld> > shuffledCombatWorldList = GetShuffledWorldList();

	//this is not total stage count in grid, but vertical stage count.
	const int32 stageCount = FMath::Pow(CurrentChapterInfo.GridSize, 2.0f);
	int32 nextCombatWorldIdx = 0;

	for (int32 stageIdx = 0; stageIdx < stageCount; stageIdx++)
	{
		//1. set stage type
		FVector2D stageCoordinate = GetStageCoordinate(stageIdx);
		UE_LOG(LogTemp, Warning, TEXT("Stage Idx : %d,  stageCoordinate : %s"), stageIdx, *stageCoordinate.ToString());
		
		if (templateIdx != -1)
		{
			stageInfo.StageType = stageTemplateInfo.StageComposition[stageIdx];
		}
		else
		{
			TArray<EStageCategoryInfo> stageTypeList = CurrentChapterInfo.StageTypeListForLevelIdx[stageCoordinate.Y + stageCoordinate.X].StageTypeList;
			checkf(stageTypeList.Num() > 0, TEXT("Stage type list data is invalid "));
			stageInfo.StageType = stageTypeList[UKismetMathLibrary::RandomInteger(stageTypeList.Num())];
		}

		//2. set stage coordinate and level
		stageInfo.Coordinate = GetStageCoordinate(stageIdx);

		//3. set default icon
		if (CurrentChapterInfo.StageIconInfoMap.Contains(stageInfo.StageType))
		{
			stageInfo.StageIcon = *CurrentChapterInfo.StageIconInfoMap.Find(stageInfo.StageType);
		}

		//4. set reward and stage's special property
		if (stageInfo.StageType == EStageCategoryInfo::E_TimeAttack) stageInfo.StageType = EStageCategoryInfo::E_Exterminate;
		else if (stageInfo.StageType == EStageCategoryInfo::E_EliteTimeAttack) stageInfo.StageType = EStageCategoryInfo::E_EliteExterminate;

		switch (stageInfo.StageType)
		{
		case EStageCategoryInfo::E_TimeAttack:
		case EStageCategoryInfo::E_EliteTimeAttack:
			/*
			nextCombatWorldIdx = (int32)(stageCoordinate.Y + stageCoordinate.X + stageCount - 1) % stageCount;
			stageInfo.TimeLimitValue = CurrentChapterInfo.EliteTimeAtkStageTimeOut;
			stageInfo.RewardInfoList = GetRewardListFromCandidates(stageInfo.StageType);
			if (stageInfo.RewardInfoList.Num() > 0)
			{
				stageInfo.StageIcon = stageInfo.RewardInfoList[0].ItemImage;
			}
			break;*/
		case EStageCategoryInfo::E_Exterminate:
		case EStageCategoryInfo::E_EliteExterminate:
		case EStageCategoryInfo::E_Escort:
		case EStageCategoryInfo::E_EliteEscort:
			nextCombatWorldIdx = (int32)(stageCoordinate.Y + stageCoordinate.X + stageCount - 1) % stageCount;
			stageInfo.RewardInfoList = GetRewardListFromCandidates(stageInfo.StageType, {});
			if (stageInfo.RewardInfoList.Num() > 0)
			{
				stageInfo.StageIcon = stageInfo.RewardInfoList[0].ItemImage;
			}
			break;
		case EStageCategoryInfo::E_Entry:
			stageInfo.RewardInfoList = GetRewardListFromCandidates(stageInfo.StageType, {});
			break;
		}

		//5. set enemy composition
		if (stageInfo.StageType == EStageCategoryInfo::E_Boss || stageInfo.StageType == EStageCategoryInfo::E_Exterminate
			|| stageInfo.StageType == EStageCategoryInfo::E_EliteExterminate || stageInfo.StageType == EStageCategoryInfo::E_TimeAttack
			|| stageInfo.StageType == EStageCategoryInfo::E_EliteTimeAttack || stageInfo.StageType == EStageCategoryInfo::E_Entry
			|| stageInfo.StageType == EStageCategoryInfo::E_EliteEscort || stageInfo.StageType == EStageCategoryInfo::E_Escort)
		{
			checkf(CurrentChapterInfo.StageEnemyCompositionInfoMap.Contains(stageInfo.StageType), TEXT("EnemyCompositionInfoMap is empty"));
			stageInfo.EnemyCompositionInfo = *CurrentChapterInfo.StageEnemyCompositionInfoMap.Find(stageInfo.StageType);
		}
		result.Add(stageInfo);
	}

	//제거 예정
	for (FVector2D& coordinate : CurrentChapterInfo.BlockedPosList)
	{
		result[GetStageIdx(coordinate)].bIsBlocked = true;
	}
	for (int32 idx : CurrentChapterInfo.BlockedStageIdxList)
	{
		result[idx].bIsBlocked = true;
	}

	return CurrentChapterInfo.StageInfoList = result;
}

TArray<FItemInfoDataStruct> UStageGeneratorComponent::GetRewardListFromCandidates(EStageCategoryInfo StageType, TArray<int32> ExceptIDList)
{
	if (!CurrentChapterInfo.StageRewardCandidateInfoMap.Contains(StageType)) return {};

	FStageRewardCandidateInfoList rewardInfoList = *CurrentChapterInfo.StageRewardCandidateInfoMap.Find(StageType);
	TArray<int32> rewardItemIDList;
	TArray<FItemInfoDataStruct> rewardItemInfoList;
	
	//pick several item id by reward candidate infos
	for (auto& rewardCandidateInfo : rewardInfoList.RewardCandidateInfoList)
	{
		if (rewardCandidateInfo.RewardItemIDList.Num() != rewardCandidateInfo.RewardItemProbabilityList.Num()) continue;

		// calculate total probability
		float totalProbability = 0.0f;
		for (float probability : rewardCandidateInfo.RewardItemProbabilityList)
		{
			totalProbability += probability;
		}

		// generate random value
		float randomValue = FMath::FRandRange(0.0f, totalProbability);

		// select reward by cumulative probability
		float cumulativeProbability = 0.0f;
		for (int32 candidateIdx = 0; candidateIdx < rewardCandidateInfo.RewardItemIDList.Num(); ++candidateIdx)
		{
			if (ExceptIDList.Contains(candidateIdx)) continue;

			cumulativeProbability += rewardCandidateInfo.RewardItemProbabilityList[candidateIdx];

			if (randomValue <= cumulativeProbability)
			{
				// selected reward item id
				int32 selectedRewardItemID = rewardCandidateInfo.RewardItemIDList[candidateIdx];

				//add item to inventory
				rewardItemIDList.Add(selectedRewardItemID);
				break;
			}
		}
	}
	
	//check duplicate item's count and add reward info list
	rewardItemIDList.Sort();
	rewardItemIDList.Add(-1); 

	int32 prevID = -1, count = 0;
	for (const int32& currID : rewardItemIDList)
	{
		if (prevID == -1) prevID = currID, count = 1;
		else if (prevID != currID)
		{
			FItemInfoDataStruct newItemInfo = GetRewardItemInfo(prevID);
			newItemInfo.ItemAmount = count;
			rewardItemInfoList.Add(newItemInfo);
			prevID = currID, count = 1;
		}	
	}
	return rewardItemInfoList;
}

FItemInfoDataStruct UStageGeneratorComponent::GetRewardItemInfo(int32 ItemID)
{
	if (!GamemodeRef.IsValid()) return FItemInfoDataStruct();
	if (ItemInfoCacheMap.Contains(ItemID)) return *ItemInfoCacheMap.Find(ItemID);

	UDataTable* itemTable = GamemodeRef.Get()->ItemInfoTable;
	if (itemTable)
	{
		FItemInfoDataStruct* itemInfoPtr = itemTable->FindRow<FItemInfoDataStruct>(FName(FString::FromInt(ItemID)), "");
		if (itemInfoPtr)
		{
			ItemInfoCacheMap.Add({ ItemID, *itemInfoPtr });
			return *itemInfoPtr;
		}
	}
	return FItemInfoDataStruct(); 
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

TArray<TSoftObjectPtr<UWorld>> UStageGeneratorComponent::GetShuffledWorldList()
{
	//Temporary code for BIC -----
	checkf(CurrentChapterInfo.StageLevelInfoMap.Contains(EStageCategoryInfo::E_Exterminate), TEXT("StageLevelInfoMap is not valid for exterminate stage"));
	FLevelSoftObjectPtrList levelInfo = *CurrentChapterInfo.StageLevelInfoMap.Find(EStageCategoryInfo::E_Exterminate);
	
	levelInfo.LevelList.Sort([this](const TSoftObjectPtr<UWorld> item1, const TSoftObjectPtr<UWorld> item2) {
		return FMath::FRand() < 0.5f;
	});
	CurrentChapterInfo.StageLevelInfoMap[EStageCategoryInfo::E_Exterminate] = levelInfo;
	return levelInfo.LevelList;
}
