// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveCollisionComponent.h"

UInteractiveCollisionComponent::UInteractiveCollisionComponent()
{
	SetCollisionProfileName("ItemTrigger", true);
}

void UInteractiveCollisionComponent::Interact()
{
	OnInteractionBegin.Broadcast();

	

	//switch (InteractionType)
	//{
	//case EInteractionType::E_CraftBox:
	//	Cast<ACraftBoxBase>(TargetActor)->OnPlayerOpenBegin.Broadcast(this);
	//case EInteractionType::E_Portal:
	//	Cast<AGateBase>(TargetActor)->EnterGate();
	//	break;
	//}
}