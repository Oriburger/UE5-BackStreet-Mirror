// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInventoryComponent.h"
#include "../CharacterBase.h"

// Sets default values for this component's properties
UItemInventoryComponent::UItemInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	InitItemInventory();
	// ...
}


// Called when the game starts
void UItemInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UItemInventoryComponent::InitItemInventory()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
}


