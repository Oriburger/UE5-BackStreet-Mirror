// Fill out your copyright notice in the Description page of Project Settings.


#include "GateBase.h"
#include "../../../Global/BackStreetGameModeBase.h"
#include "../../../Item/InteractiveCollisionComponent.h"
#include "../../AssetSystem/AssetManagerBase.h"
#include "Math/Color.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGateBase::AGateBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	OverlapVolume = CreateDefaultSubobject<UInteractiveCollisionComponent>(TEXT("OverlapVolume"));
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
	if (GamemodeRef.IsValid())
	{
		AssetManagerRef = GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();
	}
	OverlapVolume->OnInteractionBegin.AddDynamic(this, &AGateBase::EnterGate);
}

void AGateBase::InitGate_Implementation(FVector2D NewDirection, EStageCategoryInfo StageType)
{
	if (bManualMode) return;
	Direction = NewDirection;
	bIsGateActive = false;
	NextStageType = StageType;
}

void AGateBase::EnterGate()
{
	//play smash sound
	if (AssetManagerRef.IsValid())
	{
		AssetManagerRef.Get()->PlaySingleSound(this, ESoundAssetType::E_System, 3000, "Gate");
	}

	if (bManualMode)
	{
		UGameplayStatics::OpenLevel(GetWorld(), ManualTargetLevelName, true);
	}
	else
	{
		OnEnterRequestReceived.Execute(Direction);
		Destroy();
	}
}

void AGateBase::ActivateGate()
{
	bIsGateActive = true;	
}

void AGateBase::DeactivateGate()
{
	bIsGateActive = false;
}