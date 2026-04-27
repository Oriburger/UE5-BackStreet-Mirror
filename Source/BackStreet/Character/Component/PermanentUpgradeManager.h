// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "../../System/AbilitySystem/AbilityManagerBase.h"
#include "Components/ActorComponent.h"
#include "PermanentUpgradeManager.generated.h"


// dynamic delegate for 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegatePermanentUpgraded, FAbilityInfoStruct, NewUpgradeInfo); //, NewType));
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateCoreCountUpdated, int32, RemainedCoreCount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class BACKSTREET_API UPermanentUpgradeManager : public UAbilityManagerComponent
{
	GENERATED_BODY()


//======== Delegate ==============================
public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegatePermanentUpgraded OnPermanentUpgraded;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateCoreCountUpdated OnCoreCountUpdated;

//========= Basic ================================
public:	
	// Sets default values for this component's properties
	UPermanentUpgradeManager();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======== Function ================================
public: 
	UFUNCTION(BlueprintCallable)
		void InitPermanentUpgradeManager(ACharacterBase* NewCharacter, FPermanentUpgradeManagerInfo InfoOverride = FPermanentUpgradeManagerInfo());

	UFUNCTION(BlueprintCallable)
		virtual bool TryAddNewAbility(int32 AbilityID) override;

public:
	//UFUNCTION(BlueprintCallable, BlueprintPure)
		//bool GetIsUpgradeApplied(int32 UpgradeID) const { return PermanentUpgradeManagerInfo.ActiveUpgradeIDList.Contains(UpgradeID); }

protected:
	UFUNCTION()
		void ApplyUpgradeStat(FAbilityInfoStruct NewUpgradeInfo);

	UFUNCTION(BlueprintImplementableEvent)
		void OnPermanentUpgradeApplied(FAbilityInfoStruct NewANewUpgradeInfobilityInfo);

	UFUNCTION()
		void OnItemAdded(const FItemInfoDataStruct& NewItemInfo, const int32 AddCount);

	UFUNCTION(BlueprintCallable)
		virtual bool InitAbilityInfoListFromTable() override;

//======== Property ================================
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		FPermanentUpgradeManagerInfo PermanentUpgradeManagerInfo; //내부의 AbilityManagerInfo는 Super의 AbilityManagerInfo를 초기화 하는 용도

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
		class UItemInventoryComponent* OwnerInventoryComponent;

private:
	TWeakObjectPtr<class AMainCharacterBase> OwnerCharacterRef;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FPermanentUpgradeManagerInfo GetPermanentUpgradeManagerInfo();
		
	UFUNCTION(BlueprintCallable, BlueprintPure)
		int32 GetCoreCount() const { return PermanentUpgradeManagerInfo.CoreCount; }

};
