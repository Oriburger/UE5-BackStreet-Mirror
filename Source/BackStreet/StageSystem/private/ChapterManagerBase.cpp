// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/ChapterManagerBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../public/TransitionManager.h"
#include "../public/StageGenerator.h"
#include "../public/ResourceManager.h"
#include "../public/StageData.h"
#include "../public/GateBase.h"

AChapterManagerBase::AChapterManagerBase()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AChapterManagerBase::BeginPlay()
{
	Super::BeginPlay();

}

void AChapterManagerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChapterManagerBase::SetLobbyStage()
{
	TArray<AActor*> lobbyStages;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStageData::StaticClass(), lobbyStages);
	for (AActor* target : lobbyStages)
	{
		if (target->ActorHasTag(FName("Lobby")))
		{
			UE_LOG(LogTemp, Log, TEXT("AChapterManagerBase::SetLobbyStage: Find LobbyStage"));
			LobbyStage = Cast<AStageData>(target);
			Cast<AStageData>(target)->SetStageType(EStageCategoryInfo::E_Lobby);
		}
	}
}

bool AChapterManagerBase::CheckChapterClear()
{
	if (IsChapterClear())
	{
		// 챕터 클리어 관련 로직 실행
		UE_LOG(LogTemp, Log, TEXT("AChapterManagerBase::CheckChapterClear: Clear Chapter"));

		TArray<AActor*> gates;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGateBase::StaticClass(), gates);
		for (AActor* gate : gates)
		{
			AGateBase* target = Cast<AGateBase>(gate);
			if(target->ActorHasTag(FName("ChapterGate")))
				target->ActivateChapterGate();
		}
		return true;
	}
	else return false;
}

bool AChapterManagerBase::IsChapterClear()
{
	for (AStageData* stage : StageList)
	{
		if (stage->GetStageType() == EStageCategoryInfo::E_Boss)
		{
			if (stage->GetIsClear())
				return true;
		}
	}
	return false;
}

void AChapterManagerBase::MoveChapter()
{
	UE_LOG(LogTemp, Log, TEXT("AChapterManagerBase: MoveChapter"));
	ABackStreetGameModeBase* gameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(gameModeRef)) return;
	if (IsGameClear())
	{
		UE_LOG(LogTemp, Log, TEXT("MoveChapte:Game Clear"));
		ResourceManager->CleanAllResource();
		gameModeRef->FinishChapterDelegate.Broadcast(false);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MoveChapter:Move to Next Chapter"));

		ResourceManager->CleanAllResource();
		CreateChapter();
		InitChapterManager();
		// UI Update
		gameModeRef->SetMiniMapUI();
		gameModeRef->UpdateMiniMapUI();
	}


}

void AChapterManagerBase::CreateChapterManager()
{
	FActorSpawnParameters spawnParams;
	FRotator rotator;
	FVector spawnLocation = FVector::ZeroVector;

	ChapterLV = 0;
	StatWeight = 0.0f;

	StageGenerator = NewObject<UStageGenerator>(this);
	TransitionManager = NewObject<UTransitionManager>(this);


	CreateResourceManager();
	CreateChapter();
	SetLobbyStage();
	InitChapterManager();
}

void AChapterManagerBase::CreateResourceManager()
{
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	actorSpawnParameters.Owner = this;
	if (!IsValid(ResourceManagerClass)) return;
	ResourceManager = GetWorld()->SpawnActor<AResourceManager>(ResourceManagerClass,FVector(0,0,0), FRotator(0, 90, 0), actorSpawnParameters);
}

void AChapterManagerBase::InitChapterManager()
{
	// Level에 있는 초기화 필요한 Actor 초기화시키기 , 바인딩 , 및 참조 초기화 코드 등
	TransitionManager->InitTransitionManager();
	TransitionManager->InitChapter(StageList);
	CurrentStage = LobbyStage;
}

void AChapterManagerBase::CreateChapter()
{
	StageList=StageGenerator->CreateMaze();
	CurrentStage = nullptr;
	ChapterLV++;
	StatWeight += 0.1f;
}

void AChapterManagerBase::InitStartGate()
{
	TArray<AActor*> gates;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGateBase::StaticClass(), gates);
	for (AActor* gate : gates)
	{
		AGateBase* target = Cast<AGateBase>(gate);
		target->InitGate();
	}
}

void AChapterManagerBase::UpdateMapUI()
{
	Cast<ABackStreetGameModeBase>(GetOwner())->UpdateMiniMapUI();
}

