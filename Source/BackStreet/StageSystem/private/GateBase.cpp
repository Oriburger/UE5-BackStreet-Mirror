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
	bIsGateActive = true;
	if (this->ActorHasTag(FName("Tutorial")))
		DeactivateGate();

}

void AGateBase::InitGate()
{
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
	checkf(GamemodeRef.IsValid(), TEXT("GamemodeRef Is InValid"));

	CheckHaveToNeed();

	if(!MoveStageDelegate.IsBound())
		MoveStageDelegate.BindUFunction(GamemodeRef.Get()->GetChapterManagerRef()->GetTransitionManager(), FName("TryMoveStage"));
	AddGate();
	BindGateDelegate();

}

void AGateBase::BindGateDelegate()
{
	AStageData* stage = GamemodeRef.Get()->GetChapterManagerRef()->GetCurrentStage();
	checkf(IsValid(stage), TEXT("Stage Is InValid"));

	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Error, TEXT("AGateBase::BindGateDelegate -> Start Stage: %s Gate : %s Is Pending Kill"), *stage->GetName(),*this->GetName());
		return;
	}
	if (this->ActorHasTag(FName("StartGate"))) return;

	UE_LOG(LogTemp, Error, TEXT("AGateBase::BindGateDelegate -> Start Stage: %s"), *stage->GetName());
	stage->GateOffDelegate.AddDynamic(this, &AGateBase::DeactivateGate);

	
	if (this->ActorHasTag(FName("ChapterGate")))
		stage->GateOnDelegate.AddDynamic(this, &AGateBase::ActivateChapterGateAfterCheck);
	else
		stage->GateOnDelegate.AddDynamic(this, &AGateBase::ActivateGate);
	UE_LOG(LogTemp, Error, TEXT("AGateBase::BindGateDelegate -> Complete Stage: %s"),*stage->GetName());

}

void AGateBase::AddGate()
{
	AStageData* stage = GamemodeRef.Get()->GetChapterManagerRef()->GetCurrentStage();
	checkf(IsValid(stage), TEXT("Stage Is InValid"));
	stage->AddGateList(this);
}

void AGateBase::EnterGate()
{
	if (!bIsGateActive) return;

	if (this->ActorHasTag(FName("StartGate")))
		InitGate();

	RequestMoveStage();
}

void AGateBase::ActivateGate()
{
	UE_LOG(LogTemp, Log, TEXT("AGateBase:ActivateGate"));
	bIsGateActive = true;

	if (this->ActorHasTag(FName("ChapterGate")))
		ActivateChapterGateMaterial();
	else
		ActivateNormalGateMaterial();

}

void AGateBase::DeactivateGate()
{
	UE_LOG(LogTemp, Log, TEXT("AGateBase:DeactivateGate"));
	bIsGateActive = false;
	DeactivateGateMaterial();
}

void AGateBase::ActivateChapterGateAfterCheck()
{
	if (GamemodeRef.Get()->GetChapterManagerRef()->IsChapterClear())
		ActivateGate();

}

void AGateBase::ActivateChapterGateMaterial()
{
	checkf(GateMaterialList.IsValidIndex(1), TEXT("GateMaterialList[1] Is InValid"));
	Mesh->SetMaterial(0, GateMaterialList[1]);

}

void AGateBase::ActivateNormalGateMaterial()
{
	checkf(GateMaterialList.IsValidIndex(0), TEXT("GateMaterialList[0] Is InValid"));
	Mesh->SetMaterial(0, GateMaterialList[0]);

}


void AGateBase::DeactivateGateMaterial()
{
	checkf(GateMaterialList.IsValidIndex(2), TEXT("GateMaterialList[2] Is InValid"));
	Mesh->SetMaterial(0, GateMaterialList[2]);
	UE_LOG(LogTemp, Log, TEXT("AGateBase:DeactivateGate Complete"));

}

void AGateBase::RequestMoveStage()
{
	checkf(GamemodeRef.IsValid(), TEXT("GamemodeRef Is InValid"));
	if (this->ActorHasTag(FName("StartGate")))
		MoveStageDelegate.Execute(EDirection::E_Start);
	else if (this->ActorHasTag(FName("ChapterGate")))
	{
		// Need to Delete And Migrate
		if (GamemodeRef.Get()->GetChapterManagerRef()->IsChapterClear())
			MoveStageDelegate.Execute(EDirection::E_Chapter);
		else
			GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("미션을 클리어해주세요.")), FColor::White);
	
	}
	else
	{
		if (this->ActorHasTag(FName("UP")))
			MoveStageDelegate.Execute(EDirection::E_UP);
		else if (this->ActorHasTag(FName("DOWN")))
			MoveStageDelegate.Execute(EDirection::E_DOWN);
		else if (this->ActorHasTag(FName("RIGHT")))
			MoveStageDelegate.Execute(EDirection::E_RIGHT);
		else if (this->ActorHasTag(FName("LEFT")))
			MoveStageDelegate.Execute(EDirection::E_LEFT);

	}
}

void AGateBase::CheckHaveToNeed()
{
	checkf(GamemodeRef.IsValid(), TEXT("GamemodeRef Is InValid"));
	AStageData* stage = GamemodeRef.Get()->GetChapterManagerRef()->GetCurrentStage();

	checkf(IsValid(stage), TEXT("Stage Is InValid"));
	checkf(this->Tags.IsValidIndex(0), TEXT("Gate Tag[0] Is InValid"));

	if (this->Tags.Find(FName(TEXT("StartGate"))) != INDEX_NONE) return;

	if (this->Tags.Find(FName(TEXT("ChapterGate"))) != INDEX_NONE)
	{
		if (stage->GetStageCategoryType() != EStageCategoryInfo::E_Boss)
		{
			Destroy();
				return;
		}

		if (GamemodeRef.Get()->GetChapterManagerRef()->IsChapterClear())
			ActivateGate();
		else
			DeactivateGate(); 

	}
	else
	{
		checkf(this->Tags.IsValidIndex(1), TEXT("Gate Tags[1] Is InValid"));
		for (int i = 0; i < 4; i++)
		{
			if (!stage->GetGateInfoDir(EDirection(i)))
			{
				switch (i)
				{
				case 0:
					if (this->Tags.Find(FName(TEXT("UP"))) != INDEX_NONE)
					{
						Destroy();
						return;
					}
					break;
				case 1:
					if (this->Tags.Find(FName(TEXT("DOWN"))) != INDEX_NONE)
					{
						Destroy();
						return;
					}
					break;
				case 2:
					if (this->Tags.Find(FName(TEXT("LEFT"))) != INDEX_NONE)
					{
						Destroy();
						return;
					}
					break;
				case 3:
					if (this->Tags.Find(FName(TEXT("RIGHT"))) != INDEX_NONE)
					{
						Destroy();
						return;
					}
					break;
				default:
					break;
				}
	
			}
		}

		ActivateGate();
	}
}
