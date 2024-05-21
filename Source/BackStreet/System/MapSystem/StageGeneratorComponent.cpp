// Fill out your copyright notice in the Description page of Project Settings.


#include "StageGeneratorComponent.h"
#include "NewChapterManagerBase.h"

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
	//####### 선형 임시코드 ####### 
	TArray<FStageInfo> result;
	
	FStageInfo temp;
	temp.OuterLevelAssetName = CurrentChapterInfo.OuterAreaMapNameList[0];

	temp.StageType = EStageCategoryInfo::E_Entry;
	temp.TilePos = { 0, 0 };
	temp.LevelAssetName = CurrentChapterInfo.EntryStageMapNameList[0];
	temp.EnemyCompositionInfo = *CurrentChapterInfo.EnemyCompositionInfoMap.Find(EStageCategoryInfo::E_Entry);
	result.Add(temp);

	temp.StageType = EStageCategoryInfo::E_Combat;
	temp.TilePos = { 1, 0 };
	temp.LevelAssetName = CurrentChapterInfo.CombatStageMapNameList[0];
	temp.EnemyCompositionInfo = *CurrentChapterInfo.EnemyCompositionInfoMap.Find(EStageCategoryInfo::E_Combat);
	result.Add(temp);

	temp.StageType = EStageCategoryInfo::E_TimeAttack;
	temp.TilePos = { 2, 0 };
	temp.LevelAssetName = CurrentChapterInfo.TimeAttackStageMapNameList[0];
	temp.TimeLimitValue = CurrentChapterInfo.NormalTimeAtkStageTimeOut;
	temp.EnemyCompositionInfo = *CurrentChapterInfo.EnemyCompositionInfoMap.Find(EStageCategoryInfo::E_TimeAttack);
	result.Add(temp);

	temp.StageType = EStageCategoryInfo::E_Craft;
	temp.TilePos = { 3, 0 };
	temp.LevelAssetName = CurrentChapterInfo.CraftStageMapNameList[0];
	result.Add(temp);

	temp.StageType = EStageCategoryInfo::E_EliteCombat;
	temp.TilePos = { 4, 0 };
	temp.LevelAssetName = CurrentChapterInfo.CombatStageMapNameList[0];
	temp.EnemyCompositionInfo = *CurrentChapterInfo.EnemyCompositionInfoMap.Find(EStageCategoryInfo::E_EliteCombat);
	result.Add(temp);

	temp.StageType = EStageCategoryInfo::E_EliteTimeAttack;
	temp.TilePos = { 5, 0 };
	temp.LevelAssetName = CurrentChapterInfo.TimeAttackStageMapNameList[0];
	temp.TimeLimitValue = CurrentChapterInfo.EliteTimeAtkStageTimeOut;
	temp.EnemyCompositionInfo = *CurrentChapterInfo.EnemyCompositionInfoMap.Find(EStageCategoryInfo::E_EliteTimeAttack);
	result.Add(temp);

	temp.StageType = EStageCategoryInfo::E_Boss;
	temp.TilePos = { 6, 0 };
	temp.LevelAssetName = CurrentChapterInfo.BossStageMapNameList[0];
	temp.EnemyCompositionInfo = *CurrentChapterInfo.EnemyCompositionInfoMap.Find(EStageCategoryInfo::E_Boss);
	result.Add(temp);

	return result;
}
