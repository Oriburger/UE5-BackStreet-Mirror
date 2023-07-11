// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/CraftBoxBase.h"
#include "Components/SphereComponent.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"

// Sets default values
ACraftBoxBase::ACraftBoxBase()
{
    this->Tags.Add(FName("CraftingBox"));
    PrimaryActorTick.bCanEverTick = false;
    SetActorTickEnabled(false);

    // Create the trigger box component
    RootComponent = OverlapVolume = CreateDefaultSubobject<USphereComponent>(TEXT("SPHERE_COLLISION"));
    OverlapVolume->SetRelativeScale3D(FVector(7.0f));
    OverlapVolume->SetCollisionProfileName("ItemTrigger", true);
    
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_MESH"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetCollisionProfileName("Item", false);
    Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}

    // Called when the game starts or when spawned
void ACraftBoxBase::BeginPlay()
{
    Super::BeginPlay();
    OnPlayerOpenBegin.AddDynamic(this, &ACraftBoxBase::EnterUI);
}