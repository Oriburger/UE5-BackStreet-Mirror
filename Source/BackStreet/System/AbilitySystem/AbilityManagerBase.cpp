// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityManagerBase.h"
#include "../../Character/CharacterBase.h"
#include "../../System/AbilitySystem/AbilityExtraActionBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
UAbilityManagerComponent::UAbilityManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;	
}

// Called when the game starts
void UAbilityManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogAbility, Log, TEXT("UAbilityManagerComponent::BeginPlay"));

	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	OwnerCharacterRef = Cast<AMainCharacterBase>(GetOwner());
	if (!OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogAbility, Error, TEXT("UPermanentUpgradeManager::BeginPlay - Owner is not MainCharacterBase"));
	}

	//초기화 시점에 진행
	UE_LOG(LogAbility, Log, TEXT("UAbilityManagerComponent::BeginPlay - Initializing Ability Info List from DataTable"));
	InitAbilityInfoListFromTable();
}

void UAbilityManagerComponent::InitAbilityManager(ACharacterBase* NewCharacter, FAbilityManagerInfoStruct AbilityManagerInfoData)
{
	if (!IsValid(NewCharacter)) return;
	//UE_LOG(LogAbility, Log, TEXT("Initialize Ability Manager Success"));
	OwnerCharacterRef = NewCharacter;

	// 데이터 덮어쓰기
	if (AbilityManagerInfoData.AbilityInfoList.Num() > 0)
	{
		AbilityManagerInfo = AbilityManagerInfoData;
	}

	for (auto& abilityInfo : AbilityManagerInfo.ActiveAbilityInfoList)
	{
		CreateAndInitializeExtraAction(abilityInfo);
	}
}

bool UAbilityManagerComponent::TryAddNewAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid() || GetIsAbilityActive(AbilityID)
		|| AbilityManagerInfo.ActiveAbilityInfoList.Num() >= MaxAbilityCount)
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryAddNewAbility %d failed, reason : %d %d %d"), AbilityID, !OwnerCharacterRef.IsValid(), GetIsAbilityActive(AbilityID), AbilityManagerInfo.ActiveAbilityInfoList.Num() >= MaxAbilityCount);
		return false;
	}

	//추가 로직
	FAbilityInfoStruct newAbilityInfo = GetAbilityInfo(AbilityID);

	//Total Tier 계산 // 이거 왜 계산했더라
	if (!AbilityManagerInfo.AbilityTotalTier.Contains(newAbilityInfo.AbilityType))
		AbilityManagerInfo.AbilityTotalTier.Add(newAbilityInfo.AbilityType, 0);
	AbilityManagerInfo.AbilityTotalTier[newAbilityInfo.AbilityType] += (int32)newAbilityInfo.AbilityTier;

	TryUpdateCharacterStat(newAbilityInfo, false);
	AbilityManagerInfo.ActiveAbilityInfoList.Add(newAbilityInfo);
	AbilityManagerInfo.UnequippedAbilityIDSet.Remove(AbilityID);
	
	//Spawn and initialize extra action if linked
	if (newAbilityInfo.bIsRequireExtraAction)
	{
		AAbilityExtraActionBase* newExtraAction = CreateAndInitializeExtraAction(newAbilityInfo);
		if (newExtraAction == nullptr)
		{
			UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryAddNewAbility - Failed to create and initialize extra action for AbilityID %d"), AbilityID);
			return false;
		}
		newExtraAction->OnExtraActionLifeSpanEnded.AddDynamic(this, &UAbilityManagerComponent::OnExtraActionLifeSpanEnded);
		OnAbilityInfoUpdated.AddDynamic(newExtraAction, &AAbilityExtraActionBase::OnAbilityInfoUpdated);
		ExtraActionInstancePool.Add({ AbilityID, newExtraAction });
	}

	OnAbilityListUpdated.Broadcast(AbilityManagerInfo.ActiveAbilityInfoList);
	OnAbilityAdded.Broadcast(AbilityID, newAbilityInfo);

	return true;
}

bool UAbilityManagerComponent::TryRemoveAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid() || !GetIsAbilityActive(AbilityID)
		|| AbilityManagerInfo.ActiveAbilityInfoList.Num() == 0)
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryRemoveAbility %d failed, reason : %d %d %d"), AbilityID, !OwnerCharacterRef.IsValid(), !GetIsAbilityActive(AbilityID), AbilityManagerInfo.ActiveAbilityInfoList.Num() == 0);
		return false;
	}

	FCharacterGameplayInfo& ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();
	if (!ownerInfo.IsValid())
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryRemoveAbility / ownerInfo is not valid"));
		return false;
	}

	int32 targetAbilityIdx = GetAbilityListIdx(AbilityID, true);
	FAbilityInfoStruct targetAbilityInfo = AbilityManagerInfo.ActiveAbilityInfoList[targetAbilityIdx];
	if (targetAbilityIdx == -1 || !targetAbilityInfo.IsValid())
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryRemoveAbility / AbiliID is not found"));
		return false;
	}

	//Total Tier 계산
	if(!AbilityManagerInfo.AbilityTotalTier.Contains(targetAbilityInfo.AbilityType))
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryRemoveAbility / AbilityManagerInfo.AbilityTotalTier is not valid for %s"), *UEnum::GetValueAsString(targetAbilityInfo.AbilityType));
		return false;
	}
	AbilityManagerInfo.AbilityTotalTier[targetAbilityInfo.AbilityType] -= (int32)targetAbilityInfo.AbilityTier;
	TryUpdateCharacterStat(targetAbilityInfo, true);
	AbilityManagerInfo.ActiveAbilityInfoList.Remove(targetAbilityInfo);
	AbilityManagerInfo.UnequippedAbilityIDSet.Add(AbilityID);

	OnAbilityListUpdated.Broadcast(AbilityManagerInfo.ActiveAbilityInfoList);
	return true;
}

void UAbilityManagerComponent::ClearAllAbility()
{

}

bool UAbilityManagerComponent::TryUpdateCharacterStat(const FAbilityInfoStruct TargetAbilityInfo, bool bIsReset)
{
	//Validity 체크 (꺼져있는데 제거를 시도하거나, 켜져있는데 추가를 시도한다면?)
	if (bIsReset && !GetIsAbilityActive(TargetAbilityInfo.AbilityId))
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryUpdateCharacterStat / AbilityID %d is not active"), TargetAbilityInfo.AbilityId);
		return false;
	}
	
	FCharacterGameplayInfo& ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();
	if (!ownerInfo.IsValid())
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryUpdateCharacterStat / ownerInfo is not valid"));
		return false;
	}

	TArray<ECharacterStatType> targetStatTypeList; TargetAbilityInfo.TargetStatMap.GenerateKeyArray(targetStatTypeList);
	for (ECharacterStatType& statType : targetStatTypeList)
	{
		FAbilityValueInfoStruct abilityValue = TargetAbilityInfo.TargetStatMap[statType];

		if (!ownerInfo.StatGroupList.IsValidIndex((int32)statType))
		{
			UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryUpdateCharacterStat newStat for %d is not valid"), (int32)statType);
			continue;
		}

		//Adder로 동작함
		ownerInfo.SetAbilityStatInfo(statType, ownerInfo.GetAbilityStatInfo(statType) + abilityValue.Variable * (bIsReset ? -1 : 1));
		if (abilityValue.bIsContainProbability && ownerInfo.GetIsContainProbability(statType))
		{
			ownerInfo.SetProbabilityStatInfo(statType, ownerInfo.GetProbabilityStatInfo(statType) + abilityValue.ProbabilityValue * (bIsReset ? -1 : 1));
		}
		//HP UI 업데이트
		if (statType == ECharacterStatType::E_MaxHealth)
		{
			OwnerCharacterRef.Get()->TakeHeal(0.01f);
		}
		else if (statType == ECharacterStatType::E_SubWeaponCapacity)
		{
			Cast<AMainCharacterBase>(OwnerCharacterRef.Get())->SwitchWeapon(true, true);
			Cast<AMainCharacterBase>(OwnerCharacterRef.Get())->SwitchWeapon(false, true);
		}

		OnStatUpdated.Broadcast(statType, ownerInfo.GetStatGroup(statType));
	}
	return true;
}

bool UAbilityManagerComponent::GetIsAbilityActive(int32 AbilityID) const
{
	for (const FAbilityInfoStruct& abilityInfo : AbilityManagerInfo.ActiveAbilityInfoList)
	{
		if (abilityInfo.AbilityId == AbilityID)
		{
			return true;
		}
	}
	return false;
}

int32 UAbilityManagerComponent::GetMaxAbilityCount() const
{
	return MaxAbilityCount;
}

FAbilityInfoStruct UAbilityManagerComponent::GetAbilityInfo(int32 AbilityID, bool bActiveAbilityOnly)
{
	TArray<FAbilityInfoStruct> targetList = bActiveAbilityOnly ? AbilityManagerInfo.ActiveAbilityInfoList : AbilityManagerInfo.AbilityInfoList;
	for (const FAbilityInfoStruct& abilityInfo : targetList)
	{
		if (abilityInfo.AbilityId == AbilityID)
		{
			return abilityInfo;
		}
	}
	return FAbilityInfoStruct();
}


FAbilityInfoStruct& UAbilityManagerComponent::GetAbilityInfoRef(int32 AbilityID, bool bActiveAbilityOnly, bool& Result)
{
	TArray<FAbilityInfoStruct>& targetList = bActiveAbilityOnly ? AbilityManagerInfo.ActiveAbilityInfoList : AbilityManagerInfo.AbilityInfoList;
	for (FAbilityInfoStruct& abilityInfo : targetList)
	{
		if (abilityInfo.AbilityId == AbilityID)
		{
			Result = true;
			return abilityInfo;
		}
	}
	Result = false;
	return AbilityManagerInfo.AbilityInfoList[0]; //Dummy
}

TArray<int32> UAbilityManagerComponent::GetOwnedAbilityIDList()
{
	TArray<int32> returnIDList;
	for (const FAbilityInfoStruct& abilityInfo : AbilityManagerInfo.ActiveAbilityInfoList)
	{
		returnIDList.Add(abilityInfo.AbilityId);
	}
	return returnIDList;
}


int32 UAbilityManagerComponent::GetAbilityListIdx(int32 AbilityID, bool bActiveAbilityOnly)
{
	TArray<FAbilityInfoStruct> targetList = bActiveAbilityOnly ? AbilityManagerInfo.ActiveAbilityInfoList : AbilityManagerInfo.AbilityInfoList;
	for (int32 idx = 0; idx < AbilityManagerInfo.AbilityInfoList.Num(); idx++)
	{
		FAbilityInfoStruct& abilityInfo = targetList[idx];
		if (abilityInfo.AbilityId == AbilityID)
		{
			return idx;
		}
	}
	return -1;
}

bool UAbilityManagerComponent::InitAbilityInfoListFromTable()
{
	if (AbilityDataTable == nullptr)
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::InitAbilityInfoListFromTable / AbilityDataTable is null"));
		return false;
	}

	const TArray<FName> rowNameList = AbilityDataTable->GetRowNames();
	AbilityManagerInfo.AbilityInfoList.Empty();
	AbilityManagerInfo.AbilityInfoMap.Empty();
	AbilityManagerInfo.AbilityInfoList.Add(FAbilityInfoStruct()); //Dummy

	for (const FName& rowName : rowNameList)
	{
		UE_LOG(LogAbility, Log, TEXT("UAbilityManagerComponent::InitAbilityInfoListFromTable / Loading Ability Row: %s"), *rowName.ToString());
		FAbilityInfoStruct* abilityInfo = AbilityDataTable->FindRow<FAbilityInfoStruct>(rowName, "");
		if (abilityInfo != nullptr)
		{
			AbilityManagerInfo.AbilityInfoList.Add(*abilityInfo);
			AbilityManagerInfo.AbilityInfoMap.Add(abilityInfo->AbilityId, *abilityInfo);
			AbilityManagerInfo.UnequippedAbilityIDSet.Add(abilityInfo->AbilityId);
		}
	}
	return true;
}

TArray<ECharacterAbilityType> UAbilityManagerComponent::GetActiveAbilityList() const
{
	TArray<ECharacterAbilityType> returnActiveAbility;
	for (const FAbilityInfoStruct& abilityInfo : AbilityManagerInfo.ActiveAbilityInfoList)
	{
		returnActiveAbility.Add((const ECharacterAbilityType)abilityInfo.AbilityId);
	}
	return returnActiveAbility;
}

TArray<FAbilityInfoStruct> UAbilityManagerComponent::GetActiveAbilityInfoList() const
{
	return AbilityManagerInfo.ActiveAbilityInfoList;
}

AAbilityExtraActionBase* UAbilityManagerComponent::CreateAndInitializeExtraAction(FAbilityInfoStruct& NewApplicationInfo)
{
	if (NewApplicationInfo.ExtraActionClass == nullptr)
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::CreateAndInitializeExtraAction - LinkedExtraActionClass is null for AbilityID %d"), NewApplicationInfo.AbilityId);
		return nullptr;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = spawnParams.Instigator = OwnerCharacterRef.Get();
	AAbilityExtraActionBase* newExtraAction = GetWorld()->SpawnActor<AAbilityExtraActionBase>(NewApplicationInfo.ExtraActionClass, spawnParams);

	if (!IsValid(newExtraAction))
	{
		UE_LOG(LogAbility, Error, TEXT("UBattleApplicationManager::CreateAndInitializeExtraAction - Failed to spawn AbilityExtraAction for AbilityID %d"), NewApplicationInfo.AbilityId);
		return nullptr;
	}
	FAbilityInfoStruct tempInfo = NewApplicationInfo;
	newExtraAction->InitializeExtraAction(Cast<AMainCharacterBase>(OwnerCharacterRef.Get()), this, tempInfo);
	return newExtraAction;
}

void UAbilityManagerComponent::OnExtraActionLifeSpanEnded(int32 AbilityID, AAbilityExtraActionBase* ExtraActionInstance)
{
	if (ExtraActionInstancePool.Contains(AbilityID))
	{
		ExtraActionInstancePool.Remove(AbilityID);
	}
	if (IsValid(ExtraActionInstance))
	{
		ExtraActionInstance->Destroy();
	}
}