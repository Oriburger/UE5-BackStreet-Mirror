// Fill out your copyright notice in the Description page of Project Settings.


#include "NewChapterManagerBase.h"
#include "StageGeneratorComponent.h"
#include "StageManagerComponent.h"
#include "SlateBasics.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Global/BackStreetGameInstance.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
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
		}
	}
	else
	{
		UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::StartChapter, Invalid Stage Data %d"), CurrentChapterInfo.StageInfoList.Num());
	}
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

void ANewChapterManagerBase::FinishChapter(bool bChapterClear)
{
	//Set state variable
	bIsChapterFinished = true;

	OnChapterCleared.Broadcast();

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
}

void ANewChapterManagerBase::ResetChapter()
{
	//StageInfoList.Empty();
	//CurrentStageCoordinate = FVector2D(0.0f);
}

void ANewChapterManagerBase::MoveStage(int32 GateIdx)
{
	int32 stageIdx = CurrentChapterInfo.CurrentStageCoordinate + 1;
	if (!CurrentChapterInfo.StageInfoList.IsValidIndex(stageIdx)) return;
	UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::MoveStage"));
	CurrentChapterInfo.CurrentStageCoordinate = CurrentChapterInfo.CurrentStageCoordinate + 1;
	
	if (CurrentChapterInfo.StageInfoList[stageIdx].GetIsCombatStage(false)
		&& CurrentChapterInfo.StageInfoList.IsValidIndex(GateIdx))
	{
		FItemInfoDataStruct selectedReward = CurrentChapterInfo.StageInfoList[stageIdx].RewardInfoList[GateIdx];
		CurrentChapterInfo.StageInfoList[stageIdx].RewardInfoList.Empty();
		CurrentChapterInfo.StageInfoList[stageIdx].RewardInfoList.Add(selectedReward);
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
	if (bIsChapterFinished)
	{
		UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::OnStageFinished - Chapter is already finished"));
		return;
	}
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
		return;
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

void ANewChapterManagerBase::OpenMainMenuLevel()
{
	if (!GamemodeRef.IsValid())
	{
		UE_LOG(LogStage, Error, TEXT("ANewChapterManagerBase::OpenMainMenuLevel() - GamemodeRef is not valid"));
		return;
	}
	UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::OpenMainMenuLevel() - Open Main Menu Level"));
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	GamemodeRef.Get()->RequestOpenLevel("MainMenuPersistent", false);
}

void ANewChapterManagerBase::OpenNeutralZoneLevel()
{
	if (!GamemodeRef.IsValid())
	{
		UE_LOG(LogStage, Error, TEXT("ANewChapterManagerBase::OpenMainMenuLevel() - GamemodeRef is not valid"));
		return;
	}
	UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::OpenNeutralZoneLevel() - Open Neutral Zone Level"));
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	GamemodeRef.Get()->RequestOpenLevel("NeutralZonePersistent", true);
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