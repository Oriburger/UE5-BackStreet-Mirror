// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "ItemInventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UItemInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UItemInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======= Logic ============================
private:
	//init ItemInventory
	UFUNCTION()
		void InitItemInventory();

public:
	UFUNCTION(BlueprintCallable)
		void AddItem(int32 ItemID,  int32 ItemCnt);

	UFUNCTION(BlueprintCallable)
		void RemoveItem(int32 ItemID, int32 ItemCnt);

	UFUNCTION(BlueprintCallable)
		void GetItemCount(int32 ItemID);


//====== Property ===========================
private:
	TMap<FItemInventoryStruct> ItemInventoryMap;
	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;

};
