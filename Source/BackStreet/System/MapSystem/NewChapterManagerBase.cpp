// Fill out your copyright notice in the Description page of Project Settings.


#include "NewChapterManagerBase.h"
#include "StageGeneratorComponent.h"
#include "StageManagerComponent.h"
#include "SlateBasics.h"
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
}

void ANewChapterManagerBase::StartChapter(int32 NewChapterID)
{
	//init chapter with generate stage infos
	bIsChapterFinished = false;
	InitChapter(NewChapterID);
	CurrentStageLocation = FVector2D(0.0f);
	StageInfoList = StageGeneratorComponent->Generate();

	//init the first stage and start game
	if (StageInfoList.IsValidIndex(0))
	{
		StageManagerComponent->InitStage(StageInfoList[0]);
	}

	//(Temporary code) set input mode game only and hide mouse cursor
	APlayerController* playerControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(playerControllerRef))
	{
		FInputModeGameOnly gameOnlyData;
		playerControllerRef->SetInputMode(gameOnlyData);
		playerControllerRef->bShowMouseCursor = false;

		//Add combat widet to screen
		AddCombatWidget();
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
		StageManagerComponent->Initialize(CurrentChapterInfo);
	}
}

void ANewChapterManagerBase::OnStageFinished(FStageInfo StageInfo)
{
	if (bIsChapterFinished) return;
	if (!StageInfoList.IsValidIndex(CurrentStageLocation.X)) return;
	StageInfoList[CurrentStageLocation.X] = StageInfo;

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

void ANewChapterManagerBase::CreateGameResultWidget(bool bChapterClear)
{
	GameResultWidgetRef = CreateWidget(GetWorld(), bChapterClear ? ChapterClearWidgetClass : GameOverWidgetClass);
	if (IsValid(GameResultWidgetRef))
	{
		GameResultWidgetRef->AddToViewport();
	}
}

void ANewChapterManagerBase::AddCombatWidget()
{
	CombatWidgetRef = CreateWidget(GetWorld(), CombatWidgetClass);
	if (IsValid(CombatWidgetRef))
	{
		CombatWidgetRef->AddToViewport();
	}
}

void ANewChapterManagerBase::OpenMainMenuLevel()
{
	UGameplayStatics::OpenLevel(GetWorld(), "MainMenuPersistent");
}