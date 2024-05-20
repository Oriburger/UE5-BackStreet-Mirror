// Fill out your copyright notice in the Description page of Project Settings.


#include "NewChapterManagerBase.h"
#include "StageGeneratorComponent.h"
#include "StageManagerComponent.h"

// Sets default values
ANewChapterManagerBase::ANewChapterManagerBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Init Component
	StageManagerComponent = CreateDefaultSubobject<UStageManagerComponent>(TEXT("STAGE MANAGER"));
	StageGeneratorComponent = CreateDefaultSubobject<UStageGeneratorComponent>(TEXT("STAGE GENERATOR"));

	static ConstructorHelpers::FObjectFinder<UDataTable> chapterInfoTableFinder(TEXT("/Game/System/StageManager/Data/D_NewChapterInfoTable.D_NewChapterInfoTable"));
	checkf(chapterInfoTableFinder.Succeeded(), TEXT("ANewChapterManagerBase_ chapterInfoTable 탐색에 실패했습니다."));
	ChapterInfoTable = chapterInfoTableFinder.Object;
}

// Called when the game starts or when spawned
void ANewChapterManagerBase::BeginPlay()
{
	Super::BeginPlay();

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

void ANewChapterManagerBase::FinishChapter(bool bIsGameOver)
{
	if (bIsGameOver)
	{

	}
	else
	{

	}
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