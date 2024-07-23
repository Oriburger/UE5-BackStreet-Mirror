// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerComponent.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "Components/SphereComponent.h"

UCraftingManagerComponent::UCraftingManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->Tags.Add(FName("CraftingBox"));
	OnPlayerOpenBegin.AddDynamic(this, &UCraftingManagerComponent::EnterUI);
}


