// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/SphereComponent.h"
#include "CraftingManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleOpenCraftingBox, AActor*, Owner);

UCLASS()
class BACKSTREET_API UCraftingManagerComponent : public USphereComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
	FDeleOpenCraftingBox OnPlayerOpenBegin;

//======= Global =======================
public:
	UCraftingManagerComponent();

protected:
	virtual void BeginPlay() override;

//======= User Basic Function =======================
public:
	UFUNCTION(BlueprintImplementableEvent)
		void EnterUI(AActor* Causer);

////======= Default SkillUpgrade Logic =====================
//public:
//	UFUNCTION(BlueprintCallable)
//		bool AddSkill(int32 NewSkillID);
//
////======= Default SkillUpgrade Logic =====================
////SU == SkillUpgrade
//public:
//	UFUNCTION(BlueprintCallable)
//		bool UpgradeSkill(int32 NewSkillID, uint8 NewSkillLevel);
//
//	UFUNCTION(BlueprintCallable, BlueprintPure)
//		bool IsSUAvailable(int32 NewSkillID, uint8 NewSkillLevel);
//
//	UFUNCTION(BlueprintCallable, BlueprintPure)
//		bool IsValidLevelForSU(int32 NewSkillID, uint8 NewSkillLevel);
//
//	UFUNCTION(BlueprintCallable, BlueprintPure)
//		bool HasEnoughMatForSU(int32 NewSkillID, uint8 NewSkillLevel);

//======= Default WeaponUpgrade Logic =====================
public:
};
