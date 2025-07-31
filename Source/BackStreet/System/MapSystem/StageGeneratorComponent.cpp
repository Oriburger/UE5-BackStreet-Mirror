// Fill out your copyright notice in the Description page of Project Settings.


#include "StageGeneratorComponent.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "NewChapterManagerBase.h"
#include "Algo/RandomShuffle.h"
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

	UE_LOG(LogStage, Warning, TEXT("UStageGeneratorComponent::Generate() -------"));

	TArray<FStageInfo> result;
	
	FStageInfo stageInfo;
	stageInfo.OuterLevelAsset = CurrentChapterInfo.OuterStageLevelList[0];

	int32 templateIdx = -1;
	FStageTemplateInfo stageTemplateInfo;
	if (CurrentChapterInfo.StageTemplateList.Num() > 0)
	{
		templateIdx = UKismetMathLibrary::RandomInteger(CurrentChapterInfo.StageTemplateList.Num());
		stageTemplateInfo = CurrentChapterInfo.StageTemplateList[templateIdx];
		checkf(stageTemplateInfo.StageComposition.Num() == CurrentChapterInfo.StageCount, TEXT("UStageGeneratorComponent::Generate(), invalid template data"));
	}
	UE_LOG(LogStage, Warning, TEXT("UStageGeneratorComponent::Generate(), template idx : %d"), templateIdx);

	//this is not total stage count in grid, but vertical stage count.
	const int32 stageCount = CurrentChapterInfo.StageCount;
	int32 nextCombatWorldIdx = 0;

	for (int32 stageIdx = 0; stageIdx < stageCount; stageIdx++)
	{
		//1. set stage type
		int32 stageCoordinate = stageIdx;
		
		if (templateIdx != -1)
		{
			stageInfo.StageType = stageTemplateInfo.StageComposition[stageIdx];
		}
		else
		{
			TArray<EStageCategoryInfo> stageTypeList = CurrentChapterInfo.StageTypeListForLevelIdx[stageCoordinate].StageTypeList;
			checkf(stageTypeList.Num() > 0, TEXT("Stage type list data is invalid "));
			stageInfo.StageType = stageTypeList[UKismetMathLibrary::RandomInteger(stageTypeList.Num())];
		}

		TArray<TSoftObjectPtr<UWorld> > shuffledCombatWorldList = GetShuffledWorldList(stageInfo.StageType);

		//2. set stage coordinate and level
		stageInfo.Coordinate = stageIdx;
		UE_LOG(LogStage, Warning, TEXT("UStageGeneratorComponent::Generate() - Stage Idx : %d,  stageCoordinate : %d"), stageIdx, stageInfo.Coordinate);


		//3. set default icon
		if (CurrentChapterInfo.StageIconInfoMap.Contains(stageInfo.StageType))
		{
			stageInfo.StageIcon = *CurrentChapterInfo.StageIconInfoMap.Find(stageInfo.StageType);
			UE_LOG(LogStage, Warning, TEXT("UStageGeneratorComponent::Generate() - Stage Icon : %s"), *stageInfo.StageIcon->GetName());
		}
		
		//4. set reward and stage's special property
		if (stageInfo.GetIsCombatStage(false))
		{
			nextCombatWorldIdx = stageCoordinate % stageCount;
			const bool bIsFixedSkillRewardStage =	(CurrentChapterInfo.CurrentSkillRewardCount < CurrentChapterInfo.MaxSkillRewardCount
													&& CurrentChapterInfo.FixedSkillRewardStageIDList.Contains(stageIdx));
			
			stageInfo.RewardInfoList = GetRewardListFromCandidates(stageInfo.StageType, {});
			//고정 스킬 보상 스테이지를 설정한다.
			if (bIsFixedSkillRewardStage)
			{
				stageInfo.RewardInfoList.Insert(GetRewardItemInfo(CurrentChapterInfo.SkillItemID), 0);
				CurrentChapterInfo.CurrentSkillRewardCount += 1;
			}

			if (stageInfo.RewardInfoList.Num() > 0)
			{
				stageInfo.StageIcon = stageInfo.RewardInfoList[0].ItemImage;
			}
			UE_LOG(LogStage, Warning, TEXT("UStageGeneratorComponent::Generate() - Stage Type : %d, Reward Count : %d"), (int32)stageInfo.StageType, stageInfo.RewardInfoList.Num());
		}
		else if (stageInfo.StageType == EStageCategoryInfo::E_Entry)
		{
			stageInfo.RewardInfoList = GetRewardListFromCandidates(stageInfo.StageType, {});
			UE_LOG(LogStage, Warning, TEXT("UStageGeneratorComponent::Generate() - Stage Type : %d, Reward Count : %d"), (int32)stageInfo.StageType, stageInfo.RewardInfoList.Num());
		}

		//5. set enemy composition
		if (stageInfo.GetIsCombatStage())
		{
			checkf(CurrentChapterInfo.StageEnemyCompositionInfoMap.Contains(stageInfo.StageType), TEXT("EnemyCompositionInfoMap is empty"));
			stageInfo.EnemyCompositionInfo = *CurrentChapterInfo.StageEnemyCompositionInfoMap.Find(stageInfo.StageType);
		}
		UE_LOG(LogStage, Warning, TEXT("UStageGeneratorComponent::Generate() - Stage Type : %d, Enemy Composition Count : %d"), (int32)stageInfo.StageType, stageInfo.EnemyCompositionInfo.CompositionNameList.Num());
		result.Add(stageInfo);
	}

	return CurrentChapterInfo.StageInfoList = result;
}

TArray<FItemInfoDataStruct> UStageGeneratorComponent::GetRewardListFromCandidates(EStageCategoryInfo StageType, TArray<int32> ExceptIDList)
{
	if (!CurrentChapterInfo.StageRewardCandidateInfoMap.Contains(StageType)) return {};

	const FStageRewardCandidateInfoList& rewardInfoList = *CurrentChapterInfo.StageRewardCandidateInfoMap.Find(StageType);
	TArray<FItemInfoDataStruct> rewardItemInfoList;

	if (rewardInfoList.RewardCandidateInfoList.IsEmpty())
	{
		UE_LOG(LogStage, Error, TEXT("UStageGeneratorComponent::GetRewardListFromCandidates(%d), RewardCandidateInfoList is empty"), (int32)StageType);
		return {};
	}

	// 무작위 순서로 후보 리스트 순회
	TArray<FStageRewardCandidateInfo> ShuffledList = rewardInfoList.RewardCandidateInfoList;
	Algo::RandomShuffle(ShuffledList);

	TSet<int32> alreadySelectedIDs;
	if (CurrentChapterInfo.MaxSkillRewardCount <= CurrentChapterInfo.CurrentSkillRewardCount)
	{
		// 스킬 보상 횟수를 초과한 경우, 스킬 아이템은 제외
		ExceptIDList.Add(CurrentChapterInfo.SkillItemID);
	}

	for (const FStageRewardCandidateInfo& candidate : ShuffledList)
	{ 
		int32 ItemID = candidate.RewardItemID;
		float probability = candidate.RewardItemProbability;

		if (ExceptIDList.Contains(ItemID)) continue;
		if (alreadySelectedIDs.Contains(ItemID)) continue;

		float rand = FMath::FRandRange(0.0f, 1.0f);
		if (rand <= probability)
		{
			alreadySelectedIDs.Add(ItemID);

			FItemInfoDataStruct itemInfo = GetRewardItemInfo(ItemID);
			itemInfo.ItemAmount = 1;
			rewardItemInfoList.Add(itemInfo);
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

TSoftObjectPtr<UWorld> UStageGeneratorComponent::GetRandomWorld(TArray<TSoftObjectPtr<UWorld>>& WorldList)
{
	checkf(!WorldList.IsEmpty(), TEXT("WorldList Is Empty"));
	int32 idx = UKismetMathLibrary::RandomInteger(WorldList.Num());
	return WorldList[idx];
}

TArray<TSoftObjectPtr<UWorld>> UStageGeneratorComponent::GetShuffledWorldList(EStageCategoryInfo StageCategory)
{
	//Temporary code for BIC -----
	if (!CurrentChapterInfo.StageLevelInfoMap.Contains(StageCategory))
	{
		UE_LOG(LogStage, Error, TEXT("StageLevelInfoMap is not valid for exterminate stage"));
		return {};
	}
	FLevelSoftObjectPtrList levelInfo = *CurrentChapterInfo.StageLevelInfoMap.Find(StageCategory);
	
	levelInfo.LevelList.Sort([this](const TSoftObjectPtr<UWorld> item1, const TSoftObjectPtr<UWorld> item2) {
		return FMath::FRand() < 0.5f;
	});
	CurrentChapterInfo.StageLevelInfoMap[StageCategory] = levelInfo;
	return levelInfo.LevelList;
}
