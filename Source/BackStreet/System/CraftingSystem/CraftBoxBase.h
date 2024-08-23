	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "CraftBoxBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleOpenCraftingBox, AActor*, Owner);

UCLASS()
class BACKSTREET_API ACraftBoxBase : public AActor
{
	GENERATED_BODY()
// ----- Delegate ---------------------------
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDeleOpenCraftingBox OnPlayerOpenBegin;

// ----- Global, Component ------------------
public:	
	ACraftBoxBase();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UInteractiveCollisionComponent* OverlapVolume;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UTextRenderComponent* TextRenderComp;

// ------ �⺻ ���� -----------------------------
public:
	UFUNCTION(BlueprintImplementableEvent)
		void EnterUI();

	UFUNCTION(BlueprintCallable)
		void VisibleGuideTextRender(AActor* Causer);

	UFUNCTION(BlueprintCallable)
		void InVisibleGuideTextRender(AActor* Causer);
};
