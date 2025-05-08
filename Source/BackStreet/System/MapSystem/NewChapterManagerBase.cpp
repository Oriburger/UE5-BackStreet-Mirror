// Fill out your copyright notice in the Description page of Project Settings.


#include "NewChapterManagerBase.h"
#include "StageGeneratorComponent.h"
#include "StageManagerComponent.h"
#include "SlateBasics.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Global/BackStreetGameInstance.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
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
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
	GameInstanceRef = Cast<UBackStreetGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
}

void ANewChapterManagerBase::StartChapter(int32 NewChapterID)
{
	UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::StartChapter(%d)"), NewChapterID);

	//init chapter with generate stage infos
	CurrentChapterInfo.bIsChapterInitialized = true;
	InitChapter(NewChapterID);
	CurrentChapterInfo.CurrentStageCoordinate = FVector2D(0.0f);
	CurrentChapterInfo.StageInfoList = StageGeneratorComponent->Generate();
	OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);

	//init the first stage and start game
	if (CurrentChapterInfo.StageInfoList.IsValidIndex(0))
	{
		int32 stageIdx = StageGeneratorComponent->GetStageIdx(CurrentChapterInfo.CurrentStageCoordinate);
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
		int32 stageIdx = StageGeneratorComponent->GetStageIdx(CurrentChapterInfo.CurrentStageCoordinate);
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

	//Clear resource
	StageManagerComponent->ClearPreviousResource();

	OnChapterCleared.Broadcast();

	//change level to next chapter
	if (CurrentChapterInfo.ChapterID != MaxChapterID)
	{
		StartChapter(CurrentChapterInfo.ChapterID + 1);
	}
	else
	{
		//나중에 Reward UI로 대체하기
		OpenNeutralZoneLevel();
	}
}

void ANewChapterManagerBase::ResetChapter()
{
	//StageInfoList.Empty();
	//CurrentStageCoordinate = FVector2D(0.0f);
}

void ANewChapterManagerBase::MoveStage(FVector2D direction)
{
	int32 stageIdx = StageGeneratorComponent->GetStageIdx(CurrentChapterInfo.CurrentStageCoordinate + direction);
	if (!CurrentChapterInfo.StageInfoList.IsValidIndex(stageIdx)) return;
	UE_LOG(LogStage, Warning, TEXT("ANewChapterManagerBase::MoveStage,  %s  --(%s)-->  %s"), *CurrentChapterInfo.CurrentStageCoordinate.ToString(), *direction.ToString(), *(CurrentChapterInfo.CurrentStageCoordinate + direction).ToString())
	CurrentChapterInfo.CurrentStageCoordinate = CurrentChapterInfo.CurrentStageCoordinate + direction;
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

	newInfo = ChapterInfoTable->FindRow<FChapterInfo>(FName(rowName), rowName);
	if (newInfo != nullptr)
	{
		ChapterID = NewChapterID;
		CurrentChapterInfo = *newInfo;
		CurrentChapterInfo.bIsChapterInitialized = true;
		StageGeneratorComponent->InitGenerator(CurrentChapterInfo);
		StageManagerComponent->Initialize(CurrentChapterInfo);
		InitStageIconTranslationList({20, 65});
		OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);
	}
}

void ANewChapterManagerBase::OnStageFinished(FStageInfo StageInfo)
{
	if (bIsChapterFinished) return;
	int32 stageIdx = StageGeneratorComponent->GetStageIdx(StageInfo.Coordinate);
	if (!CurrentChapterInfo.StageInfoList.IsValidIndex(stageIdx)) return;
	CurrentChapterInfo.StageInfoList[stageIdx] = StageInfo;
	OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);

	//Game Over
	if (StageInfo.bIsGameOver)
	{
		//Important!) Gameresult widget must include calling the function 'BackStreetGamemode->FinishChapter' 
		CreateGameResultWidget(false);
	}
	else if (StageInfo.bIsClear)
	{
		if (StageInfo.StageType == EStageCategoryInfo::E_Boss)
		{
			//Important!) Gameresult widget must include calling the function 'BackStreetGamemode->FinishChapter'
			CreateGameResultWidget(true);
		}
	}

}

void ANewChapterManagerBase::InitStageIconTranslationList(FVector2D Threshold)
{
	TArray<FVector2D> newList; 
	for (int32 idx = 0; idx < FMath::Pow(CurrentChapterInfo.GridSize, 2.0f); idx++)
	{
		FVector2D newValue;

		newValue.X = FMath::RandRange(-Threshold.X, Threshold.X);
		newValue.Y = FMath::RandRange(-Threshold.Y, Threshold.Y);

		newList.Add(newValue);
	}
	SetStageIconTranslationList(newList);
}

FStageInfo& ANewChapterManagerBase::GetStageInfo(int32 StageIdx)
{
	checkf(CurrentChapterInfo.StageInfoList.IsValidIndex(StageIdx), TEXT("ANewChapterManagerBase::GetStageInfo(%d) is not valid idx for list %d"), StageIdx, CurrentChapterInfo.StageInfoList.Num());
	return CurrentChapterInfo.StageInfoList[StageIdx];
}

FStageInfo& ANewChapterManagerBase::GetStageInfoWithCoordinate(FVector2D StageCoordinate)
{
	return GetStageInfo(StageGeneratorComponent->GetStageIdx(StageCoordinate));
}

FName ANewChapterManagerBase::GetStageTypeName(EStageCategoryInfo StageType)
{
	switch (StageType)
	{
	case EStageCategoryInfo::E_Boss:
		return FName("E_Boss");
	case EStageCategoryInfo::E_Exterminate:
		return FName("E_Exterminate");
	case EStageCategoryInfo::E_EliteExterminate:
		return FName("E_Combat");
	case EStageCategoryInfo::E_Craft:
		return FName("E_Craft");
	case EStageCategoryInfo::E_TimeAttack:
		return FName("E_TimeAttack");
	case EStageCategoryInfo::E_EliteTimeAttack:
		return FName("E_EliteTimeAttack");
	case EStageCategoryInfo::E_Escort:
		return FName("E_Escort");
	case EStageCategoryInfo::E_EliteEscort:
		return FName("E_EliteEscort");
	case EStageCategoryInfo::E_Entry:
		return FName("E_Entry");
	case EStageCategoryInfo::E_Gatcha:
		return FName("E_Gatcha");
	case EStageCategoryInfo::E_MiniGame:
		return FName("E_MiniGame");
	}
	return FName("None");
}

bool ANewChapterManagerBase::GetIsStageBlocked(FVector2D StageCoordinate)
{
	int32 stageIdx = StageGeneratorComponent->GetStageIdx(StageCoordinate);
	if (!CurrentChapterInfo.StageInfoList.IsValidIndex(stageIdx)) return false;
	return CurrentChapterInfo.StageInfoList[stageIdx].bIsBlocked;
}

void ANewChapterManagerBase::SetTutorialCompletion(bool bCompleted)
{
	// GameInstance 가져오기
	UBackStreetGameInstance* gameInstance = Cast<UBackStreetGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		// 게임 인스턴스의 ProgressSaveData에 튜토리얼 완료 상태 저장
		FSaveSlotInfo saveSlotInfo;
		gameInstance->GetCurrentSaveSlotInfo(saveSlotInfo);
		saveSlotInfo.bIsTutorialDone = bCompleted;
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
		FProgressSaveData progressSaveData;
		gameInstance->GetCachedProgressSaveData(progressSaveData);
		return true;
		//return progressSaveData.bIsTutorialDone;
	}
	UE_LOG(LogStage, Error, TEXT("ANewChapterManagerBase::GetTutorialCompletion() - game instance is not valid"));
	return false;
}

void ANewChapterManagerBase::CreateGameResultWidget(bool bChapterClear)
{
	GameResultWidgetRef = CreateWidget(GetWorld(), bChapterClear ? ChapterClearWidgetClass : GameOverWidgetClass);
	if (IsValid(GameResultWidgetRef))
	{
		GameResultWidgetRef->AddToViewport();
	}
}

void ANewChapterManagerBase::OpenMainMenuLevel()
{
	if (!GamemodeRef.IsValid())
	{
		UE_LOG(LogStage, Error, TEXT("ANewChapterManagerBase::OpenMainMenuLevel() - GamemodeRef is not valid"));
		return;
	}
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
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	GamemodeRef.Get()->RequestOpenLevel("NeutralZonePersistent", true);
}
