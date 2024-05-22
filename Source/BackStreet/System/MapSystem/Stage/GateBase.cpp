// Fill out your copyright notice in the Description page of Project Settings.


#include "GateBase.h"
#include "StageData.h"
#include "../TransitionManager.h"
#include "../Chapter/ChapterManagerBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "Math/Color.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGateBase::AGateBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	OverlapVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapVolume"));
	OverlapVolume->SetupAttachment(RootComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(RootComponent);

	this->Tags.Add("Gate");
}

// Called when the game starts or when spawned
void AGateBase::BeginPlay()
{
	Super::BeginPlay();
	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
}

void AGateBase::InitGate(FVector2D NewDirection)
{
	Direction = NewDirection;
	bIsGateActive = false;
}

void AGateBase::EnterGate()
{
	OnEnterRequestReceived.Execute(Direction);
	Destroy();
}

void AGateBase::ActivateGate()
{
	bIsGateActive = true;	
}

void AGateBase::DeactivateGate()
{
	bIsGateActive = false;
}