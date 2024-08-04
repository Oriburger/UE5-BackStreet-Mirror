// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Global/BackStreet.h"
#include "Components/BoxComponent.h"
#include "InteractiveCollisionComponent.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	E_None					UMETA(DisplayName = "None"),
	E_Weapon				UMETA(DisplayName = "Weapon"),
	E_Item					UMETA(DisplayName = "Item"),
	E_CraftBox				UMETA(DisplayName = "CraftBox"),
	E_ResearchBox			UMETA(DisplayName = "ResearchBox"),
	E_GatchaBox 			UMETA(DisplayName = "GatchaBox"),
	E_Portal	 			UMETA(DisplayName = "Portal"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateInteract);

UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKSTREET_API UInteractiveCollisionComponent : public UBoxComponent
{
	GENERATED_BODY()

	UInteractiveCollisionComponent();

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateInteract OnInteractionBegin;

public:
	UFUNCTION()
		void Interact();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
		EInteractionType InteractionType;
	
};
