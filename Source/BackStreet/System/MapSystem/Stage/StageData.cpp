// Fill out your copyright notice in the Description page of Project Settings.
#include "StageData.h"
#include "GateBase.h"
#include "../ResourceManager.h"
#include "../Chapter/ChapterManagerBase.h"
#include "../../../Global/BackStreetGameModeBase.h"

void AStageData::BeginPlay()
{
	Super::BeginPlay();
}

AStageData::AStageData()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStageData::OpenAllGate()
{
	TArray<AGateBase*> target = GetGateList();
	for (int32 idx = GetGateList().Num() - 1; idx >= 0; idx--)
	{

		if (IsValid(target[idx]))
		{
			UE_LOG(LogTemp, Error, TEXT("AStageData::OpenAllGate -> Gate : %s Is Activate"), *GetGateList()[idx]->GetName());
			GetGateList()[idx]->ActivateGate();
		}
		
	}
}

void AStageData::CloseAllGate()
{
	UE_LOG(LogTemp, Error, TEXT("AStageData::CloseAllGate"));
	TArray<AGateBase*> target = GetGateList();
	for (int32 idx = GetGateList().Num() - 1; idx >= 0; idx--)
	{
		if (IsValid(target[idx]))
		{
			UE_LOG(LogTemp, Error, TEXT("AStageData::CloseAllGate -> Gate : %s Is Deactivate"), *GetGateList()[idx]->GetName());
			GetGateList()[idx]->DeactivateGate();
		}	
	}
}

void AStageData::DoStageClearTask()
{
	SetIsClear(true);
	Cast<AChapterManagerBase>(GetOwner())->GetResourceManager()->SpawnRewardBox(this);
	Cast<AChapterManagerBase>(GetOwner())->DoChapterClearTaskAfterCheck();
	OpenAllGate();
	if(GetStageCategoryType()!=EStageCategoryInfo::E_Boss)
		Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()))->UIAnimationDelegate.Broadcast(FName("StageClear"));

}