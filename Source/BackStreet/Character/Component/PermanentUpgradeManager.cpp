// Fill out your copyright notice in the Description page of Project Settings.


#include "./PermanentUpgradeManager.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"

// Sets default values for this component's properties
UPermanentUpgradeManager::UPermanentUpgradeManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPermanentUpgradeManager::BeginPlay()
{
	Super::BeginPlay();


	OnPermanentUpgraded.AddDynamic(this, &UPermanentUpgradeManager::OnPermanentUpgradeApplied);
}

void UPermanentUpgradeManager::InitPermanentUpgradeManager(ACharacterBase* NewCharacter, FPermanentUpgradeManagerInfo InfoOverride)
{
	InitAbilityManager(NewCharacter, InfoOverride.AbilityManagerInfo);

	OwnerInventoryComponent = Cast<UItemInventoryComponent>(NewCharacter->GetComponentByClass(UItemInventoryComponent::StaticClass()));
	if (IsValid(OwnerInventoryComponent))
	{
		OwnerInventoryComponent->OnItemAdded.AddDynamic(this, &UPermanentUpgradeManager::OnItemAdded);
	}

	if (InfoOverride.bIsInitialized)
	{
		if (bPrintDebugLog)
		{
			UE_LOG(LogPermanentGrowth, Log, TEXT("UPermanentUpgradeManager::InitPermanentUpgradeManager - Initializing with InfoOverride"));
			UE_LOG(LogPermanentGrowth, Log, TEXT("UPermanentUpgradeManager::InitPermanentUpgradeManager bIsAlreadyInitialized: %d"), PermanentUpgradeManagerInfo.bIsInitialized ? 1 : 0);
		}

		bool bIsAlreadyInitialized = PermanentUpgradeManagerInfo.bIsInitialized;
		PermanentUpgradeManagerInfo = InfoOverride;

		if (!bIsAlreadyInitialized)
		{
			for (FAbilityInfoStruct& info : PermanentUpgradeManagerInfo.AbilityManagerInfo.ActiveAbilityInfoList)
			{
				if (bPrintDebugLog)
				{
					UE_LOG(LogPermanentGrowth, Log, TEXT("Applying upgrade stat for AbilityID: %d"), info.AbilityId);
					//info.TargetStatMap.GenerateKeyArray print
					for (const TPair<ECharacterStatType, FAbilityValueInfoStruct>& pair : info.TargetStatMap)
					{
						UE_LOG(LogPermanentGrowth, Log, TEXT(" - StatType: %d, Value: %f"), (int32)pair.Key, pair.Value);
					}
				}
				ApplyUpgradeStat(info);
			}
		}
	}
	else
	{
		PermanentUpgradeManagerInfo = FPermanentUpgradeManagerInfo();
		PermanentUpgradeManagerInfo.bIsInitialized = true;
	}
}

bool UPermanentUpgradeManager::TryAddNewAbility(int32 AbilityID)
{
	if (!Super::TryAddNewAbility(AbilityID))
	{
		UE_LOG(LogPermanentGrowth, Error, TEXT("UPermanentUpgradeManager::TryAddNewAbility - Failed to add new ability with ID %d"), AbilityID);
		return false;
	}

	FAbilityInfoStruct newUpgradeInfo = GetAbilityInfo(AbilityID);
	if(newUpgradeInfo.AbilityName == NAME_None)
	{
		UE_LOG(LogPermanentGrowth, Error, TEXT("UPermanentUpgradeManager::TryAddNewAbility - AbilityID %d is not found in AbilityInfoList"), AbilityID);
		return false;
	}

	if (bPrintDebugLog)
	{
		UE_LOG(LogPermanentGrowth, Log, TEXT("\n\n=====TryApplyPermanentUpgrade(%d)======"), AbilityID);
		UE_LOG(LogPermanentGrowth, Log, TEXT("> AbilityID : %d"), AbilityID);
		UE_LOG(LogPermanentGrowth, Log, TEXT("> AbilityName : %s"), *newUpgradeInfo.AbilityName.ToString());
		UE_LOG(LogPermanentGrowth, Log, TEXT("> CoreCount : %d"), PermanentUpgradeManagerInfo.CoreCount);
		UE_LOG(LogPermanentGrowth, Log, TEXT("> RequireCost : %d"), newUpgradeInfo.RequireCost);
		UE_LOG(LogPermanentGrowth, Log, TEXT("===================================\n\n"));
	}
	
	//Core Â÷°¨
	if (PermanentUpgradeManagerInfo.CoreCount < newUpgradeInfo.RequireCost)
	{
		UE_LOG(LogPermanentGrowth, Error, TEXT("TryApplyPermanentUpgrade Failed - Not enough Core : %d / %d"), PermanentUpgradeManagerInfo.CoreCount, newUpgradeInfo.RequireCost);
		return false;
	}
	PermanentUpgradeManagerInfo.CoreCount -= newUpgradeInfo.RequireCost;
	OnCoreCountUpdated.Broadcast(PermanentUpgradeManagerInfo.CoreCount);
	OnPermanentUpgraded.Broadcast(newUpgradeInfo);

	return true;
}


void UPermanentUpgradeManager::ApplyUpgradeStat(FAbilityInfoStruct NewUpgradeInfo)
{
	TryUpdateCharacterStat(NewUpgradeInfo, false);
}

void UPermanentUpgradeManager::OnItemAdded(const FItemInfoDataStruct& NewItemInfo, const int32 AddCount)
{
	if (NewItemInfo.ItemID == CORE_ITEM_ID)
	{
		PermanentUpgradeManagerInfo.CoreCount += AddCount;
	}
}

bool UPermanentUpgradeManager::InitAbilityInfoListFromTable()
{
	return Super::InitAbilityInfoListFromTable();
}

FPermanentUpgradeManagerInfo UPermanentUpgradeManager::GetPermanentUpgradeManagerInfo()
{
	PermanentUpgradeManagerInfo.AbilityManagerInfo = AbilityManagerInfo;
	return PermanentUpgradeManagerInfo;
}

// Called every frame
void UPermanentUpgradeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

