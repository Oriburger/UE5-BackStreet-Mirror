// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/ChapterManagerBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../public/TransitionManager.h"
#include "../public/StageGenerator.h"
#include "../public/ResourceManager.h"
#include "../public/WaveManager.h"
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

void AChapterManagerBase::InitChapterManager()
{
	FActorSpawnParameters spawnParams;
	FRotator rotator;
	FVector spawnLocation = FVector::ZeroVector;

	ChapterLV = 0;
	StatWeight = 0.0f;

	StageGenerator = NewObject<UStageGenerator>(this);
	TransitionManager = NewObject<UTransitionManager>(this);
	WaveManager = NewObject<AWaveManager>(this);

	CreateResourceManager();
	CreateChapter();
	SetLobbyStage();

	ResourceManager->InitReference(WaveManager);
	WaveManager->InitReference(this, ResourceManager);

	ResetChapter();
	WaveManager->InitWaveManager(this);
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
			Cast<AStageData>(target)->SetStageCategoryType(EStageCategoryInfo::E_Lobby);
		}
	}
}

bool AChapterManagerBase::DoChapterClearTaskAfterCheck()
{
	if (IsChapterClear())
	{
		// 챕터 클리어 관련 로직 실행
		UE_LOG(LogTemp, Log, TEXT("AChapterManagerBase::DoChapterClearTaskAfterCheck: Clear Chapter"));

		TArray<AActor*> gates;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGateBase::StaticClass(), gates);
		for (AActor* gate : gates)
		{
			AGateBase* target = Cast<AGateBase>(gate);
			if (target->ActorHasTag(FName("ChapterGate")))
				target->ActivateChapterGateAfterCheck();


		}
		return true;
	}
	else return false;
}

bool AChapterManagerBase::IsChapterClear()
{
	for (AStageData* stage : StageList)
	{
		if (stage->GetStageCategoryType() == EStageCategoryInfo::E_Boss)
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
		ResetChapter();
		// UI Update
		gameModeRef->SetMiniMapUI();
		gameModeRef->UpdateMiniMapUI();
	}


}


void AChapterManagerBase::CreateResourceManager()
{
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	actorSpawnParameters.Owner = this;
	if (!IsValid(ResourceManagerClass)) return;
	ResourceManager = GetWorld()->SpawnActor<AResourceManager>(ResourceManagerClass,FVector(0,0,0), FRotator(0, 90, 0), actorSpawnParameters);
}

void AChapterManagerBase::ResetChapter()
{
	TransitionManager->InitTransitionManager();
	TransitionManager->InitChapter(StageList);
	CurrentStage = LobbyStage;
}

void AChapterManagerBase::CreateChapter()
{
	ChapterLV++;
	InitStageTypeArray();
	StageList=StageGenerator->CreateMaze();
	CurrentStage = nullptr;
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

void AChapterManagerBase::InitStageTypeArray()
{
	TArray<FStageInfoStruct*> allStageInfo;
	StageTypeTable->GetAllRows<FStageInfoStruct>(TEXT("GetStageTypeTable"), allStageInfo);
	
	NormalStageTypes.Empty();
	BossStageTypes.Empty();

	for (FStageInfoStruct* target : allStageInfo)
	{
		if (target->ChapterLevel == ChapterLV)
		{
			if (target->StageType == EStageCategoryInfo::E_Normal)
			{
				// Temporary) Debug Code Need To Fix
				//if(target->WaveType==EWaveCategoryInfo::E_TimeLimitWave)
					NormalStageTypes.Add(*target);
			}
			else if (target->StageType == EStageCategoryInfo::E_Boss)
				BossStageTypes.Add(*target);
			
		}
	}
}

void AChapterManagerBase::UpdateMapUI()
{
	Cast<ABackStreetGameModeBase>(GetOwner())->UpdateMiniMapUI();
	WaveManager->UpdateWaveUI(CurrentStage);
}

FStageInfoStruct AChapterManagerBase::GetStageTypeInfoWithType(EStageCategoryInfo Type)
{
	if (Type == EStageCategoryInfo::E_Boss)
	{
		int32 idx = FMath::RandRange(0, BossStageTypes.Num() - 1);
		return BossStageTypes[idx];
	}
	else if (Type == EStageCategoryInfo::E_Normal)
	{
		int32 idx = FMath::RandRange(0, NormalStageTypes.Num() - 1);
		return NormalStageTypes[idx];
	}
	
	return FStageInfoStruct();
}