// Fill out your copyright notice in the Description page of Project Settings.


#include "NewChapterManagerBase.h"
#include "StageGeneratorComponent.h"
#include "StageManagerComponent.h"
#include "SlateBasics.h"
#include "Runtime/UMG/Public/UMG.h"

// Sets default values
ANewChapterManagerBase::ANewChapterManagerBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Init Component
	StageManagerComponent = CreateDefaultSubobject<UStageManagerComponent>(TEXT("STAGE MANAGER"));
	StageGeneratorComponent = CreateDefaultSubobject<UStageGeneratorComponent>(TEXT("STAGE GENERATOR"));

	//Init Data table about chapter info 
	static ConstructorHelpers::FObjectFinder<UDataTable> chapterInfoTableFinder(TEXT("/Game/System/StageManager/Data/D_NewChapterInfoTable.D_NewChapterInfoTable"));
	checkf(chapterInfoTableFinder.Succeeded(), TEXT("ANewChapterManagerBase_ chapterInfoTable 탐색에 실패했습니다."));
	ChapterInfoTable = chapterInfoTableFinder.Object;

	//init widget class (game over / chapter clear)
	static ConstructorHelpers::FClassFinder<UUserWidget> chapterClearWidgetClassFinder(TEXT("/Game/UI/InGame/Blueprint/WB_GameClear"));
	checkf(chapterClearWidgetClassFinder.Succeeded(), TEXT("ANewChapterManagerBase::loadingWidget 클래스 탐색에 실패했습니다."));
	ChapterClearWidgetClass = chapterClearWidgetClassFinder.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> gameOverWidgetClassFinder(TEXT("/Game/UI/InGame/Blueprint/WB_GameOver"));
	checkf(gameOverWidgetClassFinder.Succeeded(), TEXT("ANewChapterManagerBase::loadingWidget 클래스 탐색에 실패했습니다."));
	GameOverWidgetClass = gameOverWidgetClassFinder.Class;
}

// Called when the game starts or when spawned
void ANewChapterManagerBase::BeginPlay()
{
	Super::BeginPlay();

	StageManagerComponent->OnStageFinished.AddDynamic(this, &ANewChapterManagerBase::OnStageFinished);
}

void ANewChapterManagerBase::StartChapter(int32 NewChapterID)
{
	//init chapter with generate stage infos
	InitChapter(NewChapterID);
	CurrentStageLocation = FVector2D(0.0f);
	StageInfoList = StageGeneratorComponent->Generate();

	//init stage and start game
	StageManagerComponent->InitStage(StageInfoList[0]);

	//(Temporary code) set input mode game only and hide mouse cursor
	APlayerController* playerControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(playerControllerRef))
	{
		FInputModeGameOnly gameOnlyData;
		playerControllerRef->SetInputMode(gameOnlyData);
		playerControllerRef->bShowMouseCursor = false;
	}
}

void ANewChapterManagerBase::FinishChapter(bool bChapterClear)
{
	//Chapter Reward
	// ...

	//Result Widget
	CreateGameResultWidget(true);
}

void ANewChapterManagerBase::ResetChapter()
{
	//StageInfoList.Empty();
	//CurrentStageLocation = FVector2D(0.0f);
}

void ANewChapterManagerBase::MoveStage(FVector2D direction)
{
	if (!StageInfoList.IsValidIndex(CurrentStageLocation.X + direction.X)) return;
	CurrentStageLocation = CurrentStageLocation + direction;

	StageManagerComponent->InitStage(StageInfoList[CurrentStageLocation.X]);
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
	}
}

void ANewChapterManagerBase::OnStageFinished(FStageInfo StageInfo)
{
	if (!StageInfoList.IsValidIndex(CurrentStageLocation.X)) return;
	StageInfoList[CurrentStageLocation.X] = StageInfo;

	//Game Over
	if (StageInfo.bIsGameOver)
	{
		FinishChapter(false);
	}
	else if (StageInfo.bIsClear)
	{
		if (StageInfo.StageType == EStageCategoryInfo::E_Boss)
		{
			FinishChapter(true);
		}
	}
}

void ANewChapterManagerBase::CreateGameResultWidget(bool bChapterClear)
{
	GameResultWidgetRef = CreateWidget(GetWorld(), bChapterClear ? ChapterClearWidgetClass : GameOverWidgetClass);
	if (IsValid(GameResultWidgetRef))
	{
		GameResultWidgetRef->AddToViewport();
	}
}