// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/GateBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../public/ChapterManagerBase.h"
#include "../public/TransitionManager.h"
#include "../public/StageData.h"
#include "Math/Color.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGateBase::AGateBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	OverlapVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapVolume"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	OverlapVolume->SetupAttachment(RootComponent);
	Mesh->SetupAttachment(RootComponent);

	this->Tags.Add("Gate");
}

// Called every frame
void AGateBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when the game starts or when spawned
void AGateBase::BeginPlay()
{
	Super::BeginPlay();
	if (this->ActorHasTag(FName("Off")))
		DeactivateGate();
	
}

void AGateBase::InitGate()
{
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!GamemodeRef.IsValid()) return;
	CheckHaveToActive();
	if(!MoveStageDelegate.IsBound())
		MoveStageDelegate.BindUFunction(GamemodeRef.Get()->GetChapterManagerRef()->GetTransitionManager(), FName("TryMoveStage"));
	AddGate();

}

void AGateBase::AddGate()
{
	AStageData* stage = GamemodeRef.Get()->GetChapterManagerRef()->GetCurrentStage();
	if (!IsValid(stage)) return;
	stage->AddGateList(this);
}

void AGateBase::EnterGate()
{
	if (this->ActorHasTag(FName("StartGate")))
	{
		InitGate();
	}
	if (this->ActorHasTag(FName("Off")))
		return;
	RequestMoveStage();
}

void AGateBase::ActivateChapterGate()
{
	UE_LOG(LogTemp, Log, TEXT("AGateBase:ActivateChapterGate"));
	if (!GateMaterialList.IsValidIndex(1)) return;
	Mesh->SetMaterial(0, GateMaterialList[1]);

}

void AGateBase::ActivateNormalGate()
{
	UE_LOG(LogTemp, Log, TEXT("AGateBase:ActivateNormalGate"));
	if (!GateMaterialList.IsValidIndex(0)) return;
	Mesh->SetMaterial(0, GateMaterialList[0]);

}

void AGateBase::DeactivateGate()
{
	UE_LOG(LogTemp, Log, TEXT("AGateBase:DeactivateGate"));
	if (!GateMaterialList.IsValidIndex(2)) return;
	Mesh->SetMaterial(0, GateMaterialList[2]);

}

void AGateBase::RequestMoveStage()
{
	if (!GamemodeRef.IsValid()) return;
	if (this->ActorHasTag(FName("StartGate")))
	{
		MoveStageDelegate.Execute(EDirection::E_Start);
	}
	else if (this->ActorHasTag(FName("ChapterGate")))
	{
		if (GamemodeRef.Get()->GetChapterManagerRef()->IsChapterClear())
		{
			MoveStageDelegate.Execute(EDirection::E_Chapter);
			
		}else
		{
			GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("미션을 클리어해주세요.")), FColor::White);
		}
	}
	else
	{
		 if (this->ActorHasTag(FName("UP")))
		{
			 MoveStageDelegate.Execute(EDirection::E_UP);
			
			UE_LOG(LogTemp, Log, TEXT("Up GateInfo"));
		}
		else if (this->ActorHasTag(FName("DOWN")))
		{
			 MoveStageDelegate.Execute(EDirection::E_DOWN);
		
			UE_LOG(LogTemp, Log, TEXT("Down GateInfo"));
		}
		else if (this->ActorHasTag(FName("RIGHT")))
		{
			 MoveStageDelegate.Execute(EDirection::E_RIGHT);
		
			UE_LOG(LogTemp, Log, TEXT("Right GateInfo"));
		}
		else if (this->ActorHasTag(FName("LEFT")))
		{
			 MoveStageDelegate.Execute(EDirection::E_LEFT);
			
			UE_LOG(LogTemp, Log, TEXT("Left GateInfo"));
		}

	}
}

void AGateBase::CheckHaveToActive()
{
	if (!GamemodeRef.IsValid()) return;
	AStageData* stage = GamemodeRef.Get()->GetChapterManagerRef()->GetCurrentStage();

	if (!IsValid(stage)) return;

	if (!this->Tags.IsValidIndex(0)) return;
	if (this->Tags.Find(FName(TEXT("StartGate"))) != INDEX_NONE)
	{
		return;
	}
	if (this->Tags.Find(FName(TEXT("ChapterGate"))) != INDEX_NONE)
	{
		if (stage->GetStageType() != EStageCategoryInfo::E_Boss)
		{
			Destroy();
		}
		if (GamemodeRef.Get()->GetChapterManagerRef()->IsChapterClear())
		{
			ActivateChapterGate();
		}
		else
		{
			DeactivateGate();
		}
	
	}
	else
	{
		if (!this->Tags.IsValidIndex(1)) return;
		for (int i = 0; i < 4; i++)
		{
			if (!stage->GetGateInfoDir(EDirection(i)))
			{
				switch (i)
				{
				case 0:
					if (this->Tags.Find(FName(TEXT("UP"))) != INDEX_NONE)
						Destroy();
					break;
				case 1:
					if (this->Tags.Find(FName(TEXT("DOWN"))) != INDEX_NONE)
						Destroy();
					break;
				case 2:
					if (this->Tags.Find(FName(TEXT("LEFT"))) != INDEX_NONE)
						Destroy();
					break;
				case 3:
					if (this->Tags.Find(FName(TEXT("RIGHT"))) != INDEX_NONE)
						Destroy();
					break;
				default:
					break;
				}
	
			}
		}

		ActivateNormalGate();
	}
}
