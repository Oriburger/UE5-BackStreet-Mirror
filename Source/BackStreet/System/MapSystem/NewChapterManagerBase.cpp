// Fill out your copyright notice in the Description page of Project Settings.


#include "NewChapterManagerBase.h"
#include "StageGeneratorComponent.h"
#include "StageManagerComponent.h"
#include "SlateBasics.h"
#include "../../Global/BackStreetGameModeBase.h"
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
}

void ANewChapterManagerBase::StartChapter(int32 NewChapterID)
{
	UE_LOG(LogTemp, Warning, TEXT("ANewChapterManagerBase::StartChapter(%d)"), NewChapterID);

	//init chapter with generate stage infos
	bIsChapterFinished = false;
	InitChapter(NewChapterID);
	CurrentChapterInfo.CurrentStageCoordinate = FVector2D(0.0f);
	CurrentChapterInfo.StageInfoList = StageGeneratorComponent->Generate();
	OnChapterInfoUpdated.Broadcast(CurrentChapterInfo);

	//init the first stage and start game
	if (CurrentChapterInfo.StageInfoList.IsValidIndex(0))
	{
		int32 stageIdx = StageGeneratorComponent->GetStageIdx(CurrentChapterInfo.CurrentStageCoordinate);
		StageManagerComponent->InitStage(CurrentChapterInfo.StageInfoList[stageIdx]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ANewChapterManagerBase::StartChapter, Invalid Stage Data %d"), CurrentChapterInfo.StageInfoList.Num());
	}
}

void ANewChapterManagerBase::FinishChapter(bool bChapterClear)
{
	//Set state variable
	bIsChapterFinished = true;

	//Clear resource
	UGameplayStatics::ApplyDamage(PlayerRef.Get(), 1e8, nullptr, GetOwner(), nullptr);
	StageManagerComponent->ClearResource();

	//Reward Ã³¸®
	

	//change level to main menu
	FTimerDelegate openLevelDelegate;
	openLevelDelegate.BindUFunction(this, "OpenMainMenuLevel");
	GetWorldTimerManager().SetTimer(OpenLevelDelayHandle, openLevelDelegate, 0.5f, false);
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
	UE_LOG(LogTemp, Warning, TEXT("ANewChapterManagerBase::MoveStage,  %s  --(%s)-->  %s"), *CurrentChapterInfo.CurrentStageCoordinate.ToString(), *direction.ToString(), *(CurrentChapterInfo.CurrentStageCoordinate + direction).ToString())
	CurrentChapterInfo.CurrentStageCoordinate = CurrentChapterInfo.CurrentStageCoordinate + direction;
	StageManagerComponent->InitStage(CurrentChapterInfo.StageInfoList[stageIdx]);
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
		CurrentChapterInfo = *newInfo;
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
		//Important!) Gameresult widget must include calling the function 'BackStreetGamemode->FinishGame' 
		CreateGameResultWidget(false);
	}
	else if (StageInfo.bIsClear)
	{
		if (StageInfo.StageType == EStageCategoryInfo::E_Boss)
		{
			//Important!) Gameresult widget must include calling the function 'BackStreetGamemode->FinishGame'
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

FStageInfo ANewChapterManagerBase::GetStageInfo(int32 StageIdx)
{
	if (!CurrentChapterInfo.StageInfoList.IsValidIndex(StageIdx)) return FStageInfo();
	return CurrentChapterInfo.StageInfoList[StageIdx];
}

FStageInfo ANewChapterManagerBase::GetStageInfoWithCoordinate(FVector2D StageCoordinate)
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
	UGameplayStatics::OpenLevel(GetWorld(), "MainMenuPersistent");
}