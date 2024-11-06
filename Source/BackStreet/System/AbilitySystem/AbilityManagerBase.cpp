// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityManagerBase.h"
#include "../../Character/CharacterBase.h"
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
}

void UAbilityManagerComponent::InitAbilityManager(ACharacterBase* NewCharacter)
{
	if (!IsValid(NewCharacter)) return;
	//UE_LOG(LogTemp, Warning, TEXT("Initialize Ability Manager Success"));
	OwnerCharacterRef = NewCharacter;

	//초기화 시점에 진행
	UDataTable* abilityInfoTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Character/MainCharacter/Data/D_AbilityInfoTable.D_AbilityInfoTable'"));
	if (!InitAbilityInfoListFromTable(abilityInfoTable))
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::InitAbilityManager) DataTable is not found!"));
	}
	UE_LOG(LogTemp, Warning, TEXT("InitAbilityManager %d"), AbilityPickedInfoList.Num());
}

bool UAbilityManagerComponent::TryAddNewAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (GetIsAbilityActive(AbilityID)) return false;
	if (ActiveAbilityInfoList.Num() >= MaxAbilityCount) return false;

	//이전 단계의 어빌리티가 있다면 제거부터 함
	if (AbilityID > 0 && AbilityID % 3 != 1)
	{
		if (!TryRemoveAbility(AbilityID - 1))
		{
			UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::TryAddNewAbility %d / prev ability remove failed"), AbilityID);
		}	
	}

	//추가 로직
	FAbilityInfoStruct newAbilityInfo = GetAbilityInfo(AbilityID);
	if (newAbilityInfo.bIsRepetitive)
	{	
		float variable = newAbilityInfo.VariableInfo.Num() > 0 ? newAbilityInfo.VariableInfo[0].Variable : 1.0f;
		newAbilityInfo.TimerDelegate.BindUFunction(OwnerCharacterRef.Get(), newAbilityInfo.FuncName, variable, true, AbilityID);
		OwnerCharacterRef.Get()->GetWorldTimerManager().SetTimer(newAbilityInfo.TimerHandle, newAbilityInfo.TimerDelegate, 1.0f, true);
	}
	TryUpdateCharacterStat(newAbilityInfo, false);
	ActiveAbilityInfoList.Add(newAbilityInfo);
	AbilityPickedInfoList[AbilityID] = true;
	OnAbilityUpdated.Broadcast(ActiveAbilityInfoList);

	return true;
}

bool UAbilityManagerComponent::TryRemoveAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (!GetIsAbilityActive(AbilityID)) return false;
	if (ActiveAbilityInfoList.Num() == 0) return false;

	FCharacterGameplayInfo& ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();
	if (!ownerInfo.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::TryRemoveAbility / ownerInfo is not valid"));
		return false;
	}

	int32 targetAbilityIdx = GetAbilityListIdx(AbilityID, true);
	FAbilityInfoStruct targetAbilityInfo = ActiveAbilityInfoList[targetAbilityIdx];
	if (targetAbilityIdx == -1 || !targetAbilityInfo.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::TryRemoveAbility / AbiliID is not found"));
		return false;
	}
	else
	{
		ActiveAbilityInfoList.RemoveAt(targetAbilityIdx);
		if (targetAbilityInfo.bIsRepetitive)
		{
			OwnerCharacterRef.Get()->GetWorldTimerManager().ClearTimer(targetAbilityInfo.TimerHandle);
		}
	}

	bool result = false;
	TArray<ECharacterStatType> targetStatTypeList; targetAbilityInfo.TargetStatMap.GenerateKeyArray(targetStatTypeList);
	for (ECharacterStatType& statType : targetStatTypeList)
	{
		FAbilityValueInfoStruct abilityValue = targetAbilityInfo.TargetStatMap[statType];
		if (abilityValue.Variable <= 0.0f || !ownerInfo.StatGroupList.IsValidIndex((int32)statType))
		{
			UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::TryUpdateCharacterStat newStat for %d"), (int32)statType);
			continue;
		}

		//Adder로 동작함
		ownerInfo.SetAbilityStatInfo(statType, ownerInfo.GetAbilityStatInfo(statType) - abilityValue.Variable);
		if (abilityValue.bIsContainProbability && ownerInfo.GetIsContainProbability(statType))
		{
			ownerInfo.SetProbabilityStatInfo(statType, ownerInfo.GetProbabilityStatInfo(statType) - abilityValue.ProbabilityValue);
		}
	}

	OnAbilityUpdated.Broadcast(ActiveAbilityInfoList);
	return true;
}

void UAbilityManagerComponent::ClearAllAbility()
{
	ActiveAbilityInfoList.Empty();
}

TArray<FAbilityInfoStruct> UAbilityManagerComponent::GetRandomAbilityInfoList(int32 Count, TArray<EAbilityType> TypeList)
{
	if (AbilityPickedInfoList.IsEmpty()) return {};

	TArray<FAbilityInfoStruct> selectedAbilities;
	TArray<int32> candidates, pickedIdx;

	// TypeList와 단계 확인을 통과한 후보들 필터링
	for (int32 idx = 1; idx < AbilityInfoList.Num(); ++idx)
	{
		const FAbilityInfoStruct& abilityInfo = AbilityInfoList[idx];
		if (!AbilityPickedInfoList[idx] && TypeList.Contains(abilityInfo.AbilityType) && CanPickAbility(abilityInfo.AbilityId))
		{
			candidates.Add(idx);
		}
	}

	if (candidates.Num() <= Count)
	{
		for (int32& abilityIdx : candidates)
		{
			selectedAbilities.Add(AbilityInfoList[abilityIdx]);
		}
		return selectedAbilities;
	}

	while (selectedAbilities.Num() < Count)
	{
		int32 selectedIdx = candidates[UKismetMathLibrary::RandomInteger(candidates.Num())];
		if (selectedAbilities.Contains(AbilityInfoList[selectedIdx])) continue;
		selectedAbilities.Add(AbilityInfoList[selectedIdx]);
	}
	return selectedAbilities;
}

bool UAbilityManagerComponent::TryUpdateCharacterStat(const FAbilityInfoStruct TargetAbilityInfo, bool bIsReset)
{
	//Validity 체크 (꺼져있는데 제거를 시도하거나, 켜져있는데 추가를 시도한다면?)
	if (GetIsAbilityActive(TargetAbilityInfo.AbilityId) != bIsReset) return false;
	
	FCharacterGameplayInfo& ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();
	if (!ownerInfo.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::TryUpdateCharacterStat / ownerInfo is not valid"));
		return false;
	}

	TArray<ECharacterStatType> targetStatTypeList; TargetAbilityInfo.TargetStatMap.GenerateKeyArray(targetStatTypeList);
	for (ECharacterStatType& statType : targetStatTypeList)
	{
		FAbilityValueInfoStruct abilityValue = TargetAbilityInfo.TargetStatMap[statType];

		if (abilityValue.Variable <= 0.0f || !ownerInfo.StatGroupList.IsValidIndex((int32)statType))
		{
			UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::TryUpdateCharacterStat newStat for %d is not valid"), (int32)statType);
			continue;
		}

		//Adder로 동작함
		ownerInfo.SetAbilityStatInfo(statType, ownerInfo.GetAbilityStatInfo(statType) + abilityValue.Variable);
		if (abilityValue.bIsContainProbability && ownerInfo.GetIsContainProbability(statType))
		{
			ownerInfo.SetProbabilityStatInfo(statType, ownerInfo.GetProbabilityStatInfo(statType) + abilityValue.ProbabilityValue);
		}
		//HP UI 업데이트
		if (statType == ECharacterStatType::E_MaxHealth)
		{
			OwnerCharacterRef.Get()->TakeHeal(0.0f);
		}
	}
	return true;
}

bool UAbilityManagerComponent::GetIsAbilityActive(int32 AbilityID) const
{
	for (const FAbilityInfoStruct& abilityInfo : ActiveAbilityInfoList)
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
	TArray<FAbilityInfoStruct> targetList = bActiveAbilityOnly ? ActiveAbilityInfoList : AbilityInfoList;
	for (const FAbilityInfoStruct& abilityInfo : targetList)
	{
		if (abilityInfo.AbilityId == AbilityID)
		{
			return abilityInfo;
		}
	}
	return FAbilityInfoStruct();
}

int32 UAbilityManagerComponent::GetAbilityListIdx(int32 AbilityID, bool bActiveAbilityOnly)
{
	TArray<FAbilityInfoStruct> targetList = bActiveAbilityOnly ? ActiveAbilityInfoList : AbilityInfoList;
	for (int32 idx = 0; idx < AbilityInfoList.Num(); idx++)
	{
		FAbilityInfoStruct& abilityInfo = targetList[idx];
		if (abilityInfo.AbilityId == AbilityID)
		{
			return idx;
		}
	}
	return -1;
}

bool UAbilityManagerComponent::InitAbilityInfoListFromTable(const UDataTable* AbilityInfoTable)
{
	if (AbilityInfoTable == nullptr) return false;

	const TArray<FName> rowNameList = AbilityInfoTable->GetRowNames();
	AbilityInfoList.Empty(); AbilityPickedInfoList.Empty();
	AbilityPickedInfoList.Add(false);
	AbilityInfoList.Add(FAbilityInfoStruct());
	for (const FName& rowName : rowNameList)
	{
		FAbilityInfoStruct* abilityInfo = AbilityInfoTable->FindRow<FAbilityInfoStruct>(rowName, "");
		if (abilityInfo != nullptr)
		{
			AbilityInfoList.Add(*abilityInfo);
			AbilityPickedInfoList.Add(false);
		}
	}
	return true;
}

bool UAbilityManagerComponent::CanPickAbility(int32 AbilityID)
{
	EAbilityTierType tier = GetAbilityInfo(AbilityID).AbilityTier;
	UE_LOG(LogTemp, Warning, TEXT("CanPickAbility id : %d, tier : %d,  tf? : %d"), AbilityID, tier, (int32)AbilityPickedInfoList[AbilityID]);
	if (tier == EAbilityTierType::E_Common)
		return !AbilityPickedInfoList[AbilityID]; // 1단계는 무조건 가능
	if (tier == EAbilityTierType::E_Rare)
		return AbilityPickedInfoList[AbilityID - 1] && !AbilityPickedInfoList[AbilityID];
	if (tier == EAbilityTierType::E_Legendary)
		return AbilityPickedInfoList[AbilityID - 1] && !AbilityPickedInfoList[AbilityID];
	return false;
}

TArray<ECharacterAbilityType> UAbilityManagerComponent::GetActiveAbilityList() const
{
	TArray<ECharacterAbilityType> returnActiveAbility;
	for (const FAbilityInfoStruct& abilityInfo : ActiveAbilityInfoList)
	{
		returnActiveAbility.Add((const ECharacterAbilityType)abilityInfo.AbilityId);
	}
	return returnActiveAbility;
}

TArray<FAbilityInfoStruct> UAbilityManagerComponent::GetActiveAbilityInfoList() const
{
	return ActiveAbilityInfoList;
}
