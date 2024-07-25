// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftBoxBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"

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

    TextRenderComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GuideTextRender"));
    TextRenderComp->SetupAttachment(Mesh);
    TextRenderComp->SetText(FText::FromString(TEXT("Press 'E' To Craft")));
    TextRenderComp->SetRelativeLocation(FVector(-70, 0, 240));
    TextRenderComp->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0,0,90)));
    TextRenderComp->SetVisibility(false);
}

    // Called when the game starts or when spawned
void ACraftBoxBase::BeginPlay()
{
    Super::BeginPlay();
    OnPlayerOpenBegin.AddDynamic(this, &ACraftBoxBase::EnterUI);
}

void ACraftBoxBase::VisibleGuideTextRender(AActor* Causer)
{
    if(!IsValid(Causer)) return;
    if (Causer->ActorHasTag("Player"))
    {
        TextRenderComp->SetVisibility(true);
    }
}

void ACraftBoxBase::InVisibleGuideTextRender(AActor* Causer)
{
    if (!IsValid(Causer)) return;
    if (Causer->ActorHasTag("Player"))
    {
        TextRenderComp->SetVisibility(false);
    }
}