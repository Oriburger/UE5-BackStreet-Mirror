// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveCollisionComponent.h"

UInteractiveCollisionComponent::UInteractiveCollisionComponent()
{
	SetCollisionProfileName("ItemTrigger", true);
}

void UInteractiveCollisionComponent::Interact()
{
	OnInteractionBegin.Broadcast();
}

void UInteractiveCollisionComponent::SetInteractState(bool bNewState)
{
	bCanInteract = bNewState; 
	OnInteractionAvailableStateUpdate.Broadcast(bNewState);
}