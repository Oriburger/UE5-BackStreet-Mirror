// Fill out your copyright notice in the Description page of Project Settings.


#include "NewChapterManagerBase.h"
#include "StageGeneratorComponent.h"
#include "StageManagerComponent.h"
#include "SlateBasics.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Global/BackStreetGameInstance.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"
#include "../../System/SaveSystem/SaveSlotManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/UMG/Public/UMG.h"

// Sets default values
ANewChapterManagerBase::ANewChapterManagerBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Init Component
	StageManagerComponent = CreateDefaultSubobject<UStageManagerComponent>(TEXT("STAGE MANAGER"));
	StageGeneratorComponent = CreateDefaultSubobject<UStageGeneratorComponent>(TEXT("STAGE GENERATOR"));
}

// Called when the game starts or when spawned
void ANewChapterManagerBase::BeginPlay()
{
	Super::BeginPlay();

	StageManagerComponent->OnStageFinished.AddDynamic(this, &ANewChapterManagerBase::OnStageFinished);
	StageManagerComponent->OnStageLoadBegin.AddDynamic(this, &ANewChapterManagerBase::PauseChapterTimer);
	StageManagerComponent->OnStageLoadDone.AddDynamic(this, &ANewChapterManagerBase::UnPauseChapterTimer);
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	PlayerRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void ANewChapterManagerBase::StartChapter(int32 NewChapterID)
{
	UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::StartChapter(%d)"), NewChapterID);
	
	//Clear resource
	StageManagerComponent->ClearPreviousResource();

	//init chapter with generate stage infos
	CurrentChapterInfo.bIsChapterInitialized = true;
	InitChapter(NewChapterID);
	CurrentChapterInfo.CurrentStageCoordinate = 0;
	CurrentChapterInfo.StageInfoList = StageGeneratorComponent->Generate();
	OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);

	//init the first stage and start game
	if (CurrentChapterInfo.StageInfoList.IsValidIndex(0))
	{
		int32 stageIdx = CurrentChapterInfo.CurrentStageCoordinate;
		StageManagerComponent->InitStage(CurrentChapterInfo.StageInfoList[stageIdx]);

		//MainCharacter 730 Crash 방지
		AMainCharacterBase* playerRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if(IsValid(playerRef))
		{
			OnChapterCleared.AddDynamic(playerRef, &AMainCharacterBase::ClearAllTimerHandle);
			OnChapterOvered.AddDynamic(playerRef, &AMainCharacterBase::ClearAllTimerHandle);
		}
	}
	else
	{
		UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::StartChapter, Invalid Stage Data %d"), CurrentChapterInfo.StageInfoList.Num());
	}
	OnChapterStarted.Broadcast();
}

void ANewChapterManagerBase::ContinueChapter()
{
	UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::ContinueChapter(%d)"), CurrentChapterInfo.ChapterID);
	
	//init chapter with generate stage info
	OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);

	//init the first stage and start game
	if (CurrentChapterInfo.StageInfoList.IsValidIndex(0))
	{
		int32 stageIdx = CurrentChapterInfo.CurrentStageCoordinate;
			StageManagerComponent->InitStage(StageManagerComponent->GetCurrentStageInfo());
	}
	else
	{
		UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::ContinueChapter, Invalid Stage Data %d"), CurrentChapterInfo.StageInfoList.Num());
	}
}

void ANewChapterManagerBase::FinishChapter(bool bIsChapterCleared)
{
	//Set state variable
	bIsChapterFinished = true;
	
	//===========================================================
	//Important!) Gameresult widget must include calling the function 'BackStreetGamemode->FinishChapter' 

	//Game Over
	if (StageManagerComponent->GetCurrentStageInfo().bIsGameOver)
	{
		CreateGameResultWidget(false);
	}
	//Chapter & Game Clear
	else if (StageManagerComponent->GetCurrentStageInfo().bIsClear 
		&& StageManagerComponent->GetCurrentStageInfo().StageType == EStageCategoryInfo::E_Boss)
	{
		if (CurrentChapterInfo.ChapterID == MaxChapterID)
		{
			CreateGameResultWidget(true);
		}
		else
		{
			CreateChapterCleartWidget();
		}
	}

	//Grant core reward
	if ((StageManagerComponent->GetCurrentStageInfo().bIsClear && CurrentChapterInfo.ChapterID == MaxChapterID)
		|| StageManagerComponent->GetCurrentStageInfo().bIsGameOver)
	{
		const int32 rewardCoreCount = GetRewardCoreCount();
		PlayerRef->ItemInventory->AddItem(2, rewardCoreCount); //2 : Core itemID
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Reward Core Count : %d"), rewardCoreCount));
	}

	if (bIsChapterCleared)
	{
		UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::FinishChapter - Chapter Cleared ##############################"));
		OnChapterCleared.Broadcast();
	}
	else
	{
		UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::FinishChapter - Chapter Overed ##############################"));
		OnChapterOvered.Broadcast();
	}
}

void ANewChapterManagerBase::ResetChapter()
{
	//StageInfoList.Empty();
	//CurrentStageCoordinate = FVector2D(0.0f);
}

void ANewChapterManagerBase::MoveStage(int32 GateIdx, int32 StageIdxOverride)
{
	int32 stageIdx = CurrentChapterInfo.CurrentStageCoordinate + 1;
	if(StageIdxOverride >= 0)
	{
		stageIdx = StageIdxOverride;
	}

	if (!CurrentChapterInfo.StageInfoList.IsValidIndex(stageIdx)) return;
	UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::MoveStage"));
	CurrentChapterInfo.CurrentStageCoordinate = CurrentChapterInfo.CurrentStageCoordinate + 1;
	
	if (CurrentChapterInfo.StageInfoList[stageIdx].GetIsCombatStage(false)
		&& CurrentChapterInfo.StageInfoList.IsValidIndex(GateIdx))
	{
		CurrentChapterInfo.StageInfoList[stageIdx].MainRewardInfo = CurrentChapterInfo.StageInfoList[stageIdx].MainRewardInfoCandidateList[GateIdx];
		CurrentChapterInfo.StageInfoList[stageIdx].SubRewardInfo = CurrentChapterInfo.StageInfoList[stageIdx].SubRewardInfoCandidateList[GateIdx];
	}
	else
	{
		UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::MoveStage - Invalid GateIdx for reward %d"), GateIdx);
	}
	
	StageManagerComponent->InitStage(CurrentChapterInfo.StageInfoList[stageIdx]);
	OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);
}

void ANewChapterManagerBase::OverwriteChapterInfo(FChapterInfo NewChapterInfo, FStageInfo NewStageInfo)
{
	UE_LOG(LogSaveSystem, Log, TEXT("ANewChapterManagerBase::OverwriteChapterInfo - ChapterID : %d"), NewChapterInfo.ChapterID);
	CurrentChapterInfo = NewChapterInfo;
	StageGeneratorComponent->InitGenerator(CurrentChapterInfo);
	StageManagerComponent->OverwriteStageInfo(NewChapterInfo, NewStageInfo);
	OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);
}

int32 ANewChapterManagerBase::GetRewardCoreCount()
{
	/* @@@ 임시 @@@
	* 클리어) 챕터1 : 10개  챕터2 : 20개  챕터3 : 35개  최종 : 55개
	* 오버) 챕터1 : 5개   챕터2 : 10개  챕터3 : 20개  챕터4 : 35개
	*/

	const TArray<int32> clearRewardList = { 70, 100, 150, 175, 200 };
	if (CurrentChapterInfo.ChapterID - 1 < 0 || CurrentChapterInfo.ChapterID - 1 >= clearRewardList.Num())
	{
		UE_LOG(LogStage, Error, TEXT("ANewChapterManagerBase::GetRewardCoreCount - Invalid ChapterID %d"), CurrentChapterInfo.ChapterID);
		return 0;
	}
	
	//스테이지가 2개 이하일때는 클리어 보상의 절반(임시코드)
	const float halfCheck = (StageManagerComponent->GetCurrentStageInfo().Coordinate <= 2) ? 0.2f : 1.0f;

	if (StageManagerComponent->GetCurrentStageInfo().bIsGameOver)
	{
		return clearRewardList[CurrentChapterInfo.ChapterID - 1] * halfCheck;
	}
	else if (StageManagerComponent->GetCurrentStageInfo().bIsClear
		&& StageManagerComponent->GetCurrentStageInfo().StageType == EStageCategoryInfo::E_Boss)
	{
		return clearRewardList[CurrentChapterInfo.ChapterID] * halfCheck;
	}
	return 0;
}

void ANewChapterManagerBase::InitChapter(int32 NewChapterID)
{
	if (ChapterInfoTable == nullptr) return;

	//Load chapter info from data table
	FChapterInfo* newInfo = nullptr;
	FString rowName = FString::FromInt(NewChapterID);
	const float elapsedTime = CurrentChapterInfo.TotalElapsedTime;

	newInfo = ChapterInfoTable->FindRow<FChapterInfo>(FName(rowName), rowName);
	if (newInfo != nullptr)
	{
		ChapterID = NewChapterID;
		CurrentChapterInfo = *newInfo;
		CurrentChapterInfo.TotalElapsedTime = elapsedTime;
		bIsChapterFinished = false;
		CurrentChapterInfo.bIsChapterInitialized = true;
		StageGeneratorComponent->InitGenerator(CurrentChapterInfo);
		StageManagerComponent->Initialize(CurrentChapterInfo);
		InitStageIconTranslationList({20, 65});
		OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);
	}
	CurrentSkillRewardCount = 0;
}

void ANewChapterManagerBase::OnStageFinished(FStageInfo StageInfo)
{
	int32 stageIdx = StageInfo.Coordinate;
	if (!CurrentChapterInfo.StageInfoList.IsValidIndex(stageIdx))
	{
		UE_LOG(LogStage, Error, TEXT("ANewChapterManagerBase::OnStageFinished - Invalid StageIdx %d for StageInfoList %d"), stageIdx, CurrentChapterInfo.StageInfoList.Num());
		return;
	}
	CurrentChapterInfo.StageInfoList[stageIdx] = StageInfo;
	OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);

	//Chapter Clear
	if (StageInfo.StageType == EStageCategoryInfo::E_Boss)
	{
		FinishChapter(StageInfo.bIsClear);
	}
	else if(StageInfo.bIsGameOver)
	{
		FinishChapter(false);
	}

	return;
}

void ANewChapterManagerBase::InitStageIconTranslationList(FVector2D Threshold)
{
	/*
	TArray<FVector2D> newList; 
	for (int32 idx = 0; idx < CurrentChapterInfo.StageCount; idx++)
	{
		FVector2D newValue;

		newValue.X = FMath::RandRange(-Threshold.X, Threshold.X);
		newValue.Y = FMath::RandRange(-Threshold.Y, Threshold.Y);

		newList.Add(newValue);
	}
	SetStageIconTranslationList(newList);
	*/
}

FStageInfo& ANewChapterManagerBase::GetStageInfo(int32 StageIdx)
{
	checkf(CurrentChapterInfo.StageInfoList.IsValidIndex(StageIdx), TEXT("ANewChapterManagerBase::GetStageInfo(%d) is not valid idx for list %d"), StageIdx, CurrentChapterInfo.StageInfoList.Num());
	return CurrentChapterInfo.StageInfoList[StageIdx];
}

FName ANewChapterManagerBase::GetStageTypeName(EStageCategoryInfo StageType)
{
	switch (StageType)
	{
	case EStageCategoryInfo::E_None:
		return FName("E_None");
	case EStageCategoryInfo::E_Entry:
		return FName("E_Entry");
	case EStageCategoryInfo::E_Boss:
		return FName("E_Boss");
	case EStageCategoryInfo::E_Craft:
		return FName("E_Craft");
	case EStageCategoryInfo::E_Easy:
		return FName("E_Easy");
	case EStageCategoryInfo::E_Normal:
		return FName("E_Normal");
	case EStageCategoryInfo::E_Hard:
		return FName("E_Hard");
	case EStageCategoryInfo::E_Expert:
		return FName("E_Expert");
	case EStageCategoryInfo::E_Nightmare:
		return FName("E_Nightmare");
	}
	return FName("None");
}

void ANewChapterManagerBase::SetTutorialCompletion(bool bCompleted)
{
	// GameInstance 가져오기
	UBackStreetGameInstance* gameInstance = Cast<UBackStreetGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		// 게임 인스턴스의 ProgressSaveData에 튜토리얼 완료 상태 저장
		ASaveSlotManager* saveSlotManager = gameInstance->GetSafeSaveSlotManager();
		if (IsValid(saveSlotManager))
		{
			saveSlotManager->SetTutorialCompletion(bCompleted);
		}
		return;
	}
	UE_LOG(LogStage, Error, TEXT("ANewChapterManagerBase::SetTutorialCompletion(%d) - game instance is not valid"), (int32)bCompleted);
}

bool ANewChapterManagerBase::GetTutorialCompletion()
{
	UBackStreetGameInstance* gameInstance = Cast<UBackStreetGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		// 게임 인스턴스의 ProgressSaveData에 튜토리얼 완료 상태 저장
		FSaveSlotInfo saveSlotInfo;
		gameInstance->GetCurrentSaveSlotInfo(saveSlotInfo);
		return saveSlotInfo.bIsTutorialDone;
	}
	UE_LOG(LogStage, Error, TEXT("ANewChapterManagerBase::GetTutorialCompletion() - game instance is not valid"));
	return false;
}

void ANewChapterManagerBase::PauseChapterTimer()
{
	GetWorldTimerManager().PauseTimer(ChapterTimerHandle);
}

void ANewChapterManagerBase::UnPauseChapterTimer()
{
	if (!GetWorldTimerManager().IsTimerActive(ChapterTimerHandle))
	{
		GetWorldTimerManager().SetTimer(ChapterTimerHandle, this, &ANewChapterManagerBase::UpdateChapterElapsedTime, 0.01f, true, 0.0f);
		return;
	}
	GetWorldTimerManager().UnPauseTimer(ChapterTimerHandle);
}

void ANewChapterManagerBase::UpdateChapterElapsedTime()
{
	CurrentChapterInfo.TotalElapsedTime += 0.01f;
}