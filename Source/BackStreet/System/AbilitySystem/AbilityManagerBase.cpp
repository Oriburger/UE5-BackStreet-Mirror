// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityManagerBase.h"
#include "../../Character/CharacterBase.h"
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

	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	UpdateCumulativeProbabilityList();
}

int32 UAbilityManagerComponent::GetAbilityUpgradeCost(EAbilityTierType TargetTier)
{
	if (!AbilityManagerInfo.UpgradeCostInfoMap.Contains(TargetTier)) return -1;
	return AbilityManagerInfo.UpgradeCostInfoMap[TargetTier];
}

void UAbilityManagerComponent::InitAbilityManager(ACharacterBase* NewCharacter, FAbilityManagerInfoStruct AbilityManagerInfoData)
{
	if (!IsValid(NewCharacter)) return;
	//UE_LOG(LogAbility, Log, TEXT("Initialize Ability Manager Success"));
	OwnerCharacterRef = NewCharacter;

	//초기화 시점에 진행
	if (!InitAbilityInfoListFromTable())
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::InitAbilityManager) DataTable is not found!"));
	}
	UE_LOG(LogAbility, Log, TEXT("InitAbilityManager %d"), AbilityManagerInfo.AbilityPickedInfoList.Num());


	// 데이터 덮어쓰기
	if (AbilityManagerInfoData.AbilityInfoList.Num() > 0)
	{
		AbilityManagerInfo = AbilityManagerInfoData;
	}
}

bool UAbilityManagerComponent::TryAddNewAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (GetIsAbilityActive(AbilityID)) return false;
	if (AbilityManagerInfo.ActiveAbilityInfoList.Num() >= AbilityManagerInfo.MaxAbilityCount) return false;

	//이전 단계의 어빌리티가 있다면 제거부터 함 // not working 250107 
	if (AbilityID > 0 && AbilityID % 3 != 1)
	{
		if (!TryRemoveAbility(AbilityID - 1))
		{
			UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryAddNewAbility %d / prev ability remove failed"), AbilityID);
		}	
	}

	//추가 로직
	FAbilityInfoStruct newAbilityInfo = GetAbilityInfo(AbilityID);
	if (newAbilityInfo.bIsRepetitive && !newAbilityInfo.FuncName.IsNone())
	{	
		float variable = newAbilityInfo.VariableInfo.Num() > 0 ? newAbilityInfo.VariableInfo[0].Variable : 1.0f;
		newAbilityInfo.TimerDelegate.BindUFunction(OwnerCharacterRef.Get(), newAbilityInfo.FuncName, variable, true, AbilityID);
		OwnerCharacterRef.Get()->GetWorldTimerManager().SetTimer(newAbilityInfo.TimerHandle, newAbilityInfo.TimerDelegate, 1.0f, true);
	}
	//Total Tier 계산
	if (!AbilityTotalTier.Contains(newAbilityInfo.AbilityType))
		AbilityTotalTier.Add(newAbilityInfo.AbilityType, 0);
	AbilityTotalTier[newAbilityInfo.AbilityType] += (int32)newAbilityInfo.AbilityTier;
	UE_LOG(LogAbility, Log, TEXT(""))

	TryUpdateCharacterStat(newAbilityInfo, false);
	AbilityManagerInfo.ActiveAbilityInfoList.Add(newAbilityInfo);
	AbilityManagerInfo.AbilityPickedInfoList[(AbilityID - 1) / 3] = true;
	OnAbilityUpdated.Broadcast(AbilityManagerInfo.ActiveAbilityInfoList);

	return true;
}

bool UAbilityManagerComponent::TryRemoveAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (!GetIsAbilityActive(AbilityID)) return false;
	if (AbilityManagerInfo.ActiveAbilityInfoList.Num() == 0) return false;

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
	else
	{
		AbilityManagerInfo.ActiveAbilityInfoList.RemoveAt(targetAbilityIdx);
		if (targetAbilityInfo.bIsRepetitive)
		{
			OwnerCharacterRef.Get()->GetWorldTimerManager().ClearTimer(targetAbilityInfo.TimerHandle);
		}
	}

	//Total Tier 계산
	AbilityTotalTier[targetAbilityInfo.AbilityType] -= (int32)targetAbilityInfo.AbilityTier;
	TryUpdateCharacterStat(targetAbilityInfo, false);

	OnAbilityUpdated.Broadcast(AbilityManagerInfo.ActiveAbilityInfoList);
	return true;
}

bool UAbilityManagerComponent::TryUpgradeAbility(int32 AbilityID)
{
	for (auto& abilityInfo : AbilityManagerInfo.ActiveAbilityInfoList)
	{
		if (AbilityID == abilityInfo.AbilityId)
		{
			if (abilityInfo.AbilityTier == EAbilityTierType::E_Legendary) return false; //최대 티어인 경우 업그레이드 불가

			// 새 정보를 덮어쓰고, Total Tier를 업데이트
			AbilityTotalTier[abilityInfo.AbilityType] -= (int32)abilityInfo.AbilityTier;
			abilityInfo = GetAbilityInfo(AbilityID + 1);
			AbilityTotalTier[abilityInfo.AbilityType] += (int32)abilityInfo.AbilityTier;

			TryUpdateCharacterStat(abilityInfo, false);
			return true;
		}
	}
	return false; // AbilityID가 ActiveAbilityInfoList에 존재하지 않는 경우
}

void UAbilityManagerComponent::ClearAllAbility()
{
	AbilityManagerInfo.ActiveAbilityInfoList.Empty();
}

void UAbilityManagerComponent::UpdateProbilityValue(EAbilityTierType TierType, float NewProbability)
{
	if (!AbilityManagerInfo.ProbabilityInfoMap.Contains(TierType))
	{
		AbilityManagerInfo.ProbabilityInfoMap.Add(TierType, NewProbability);
	}
	else
	{
		AbilityManagerInfo.ProbabilityInfoMap[TierType] = NewProbability;
	}
	UpdateCumulativeProbabilityList();
}

TArray<FAbilityInfoStruct> UAbilityManagerComponent::GetRandomAbilityInfoList(int32 Count, TArray<EAbilityType> TypeList)
{
	if (AbilityManagerInfo.AbilityPickedInfoList.IsEmpty())
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList, AbilityPickedInfoList is empty"));
		return {};
	}
	
	TArray<FAbilityInfoStruct> selectedAbilities;
	TArray<int32> candidateGroups, pickedGroupIdx;
	
	//무한 루프 방지 코드
	int32 tryCount = 0; 
	const int32 tryCountThreshold = Count * 100;

	// TypeList와 단계 확인을 통과한 후보들 필터링
	for (int32 groupIdx = 0; groupIdx < AbilityManagerInfo.AbilityInfoList.Num() / 3; ++groupIdx)
	{
		const FAbilityInfoStruct& abilityInfo = AbilityManagerInfo.AbilityInfoList[groupIdx * 3 + 1];
		if (TypeList.Contains(abilityInfo.AbilityType) && CanPickAbility(abilityInfo.AbilityId))
		{
			candidateGroups.Add(groupIdx);
		}
	}
	if (candidateGroups.IsEmpty())
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList, candidateGroups is empty"));
		return {};
	}
	
	while (selectedAbilities.Num() < Count)
	{
		tryCount += 1;
		if (tryCount >= tryCountThreshold) break;

		int32 randomCandidateIdx = UKismetMathLibrary::RandomInteger(candidateGroups.Num());
		int32 selectedGroupIdx = candidateGroups[randomCandidateIdx];
		int32 resultAbilityIdx = -1;

		if (pickedGroupIdx.Contains(selectedGroupIdx))
		{
			continue;
		}

		UE_LOG(LogAbility, Log, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList Loop) Count : %d, selected : %d, left : %d"), Count, selectedAbilities.Num(), candidateGroups.Num());

		//만약 잔여 어빌리티가 부족하다면?
		if (candidateGroups.Num() < Count - selectedAbilities.Num())
		{
			UE_LOG(LogAbility, Log, TEXT("Not enough left abilities"));
			for (auto& candidate : candidateGroups)
			{
				selectedGroupIdx = candidate;
				resultAbilityIdx = GetRandomAbilityIdxUsingGroupIdx(selectedGroupIdx);
				if (resultAbilityIdx > 0 && AbilityManagerInfo.AbilityInfoList.IsValidIndex(resultAbilityIdx) && AbilityManagerInfo.AbilityInfoList[resultAbilityIdx].AbilityId > 0)
				{
					selectedAbilities.Add(AbilityManagerInfo.AbilityInfoList[resultAbilityIdx]);
					UE_LOG(LogAbility, Log, TEXT("-> Picked"));
					pickedGroupIdx.Add(selectedGroupIdx);
				}
				else
				{
					UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList(%d) auto fill error - selected Group %d / resultAbilityIdx %d / Valid Idx : %d"), Count, selectedGroupIdx, resultAbilityIdx, (int32)AbilityManagerInfo.AbilityInfoList.IsValidIndex(resultAbilityIdx));
					if (AbilityManagerInfo.AbilityInfoList.IsValidIndex(resultAbilityIdx))
					{
						UE_LOG(LogAbility, Log, TEXT("ㄴ> Ability Id : %d"), AbilityManagerInfo.AbilityInfoList[resultAbilityIdx].AbilityId);
					}
				}
			}
			break;
		}
		else
		{
			//확률 프로퍼티가 지정되어있지 않을 경우
			resultAbilityIdx = GetRandomAbilityIdxUsingGroupIdx(selectedGroupIdx);
			if (resultAbilityIdx > 0 && AbilityManagerInfo.AbilityInfoList.IsValidIndex(resultAbilityIdx) && AbilityManagerInfo.AbilityInfoList[resultAbilityIdx].AbilityId > 0)
			{
				selectedAbilities.Add(AbilityManagerInfo.AbilityInfoList[resultAbilityIdx]);
				pickedGroupIdx.Add(selectedGroupIdx);
				candidateGroups.RemoveAtSwap(randomCandidateIdx);
			}
			else
			{
				UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList(%d) error with cumulativeprobList - selected Group %d / resultAbilityIdx %d / Valid Idx : %d"), Count, selectedGroupIdx, resultAbilityIdx, (int32)AbilityManagerInfo.AbilityInfoList.IsValidIndex(resultAbilityIdx));
			}
		}
	}
	
	UE_LOG(LogAbility, Log, TEXT("Total Try Count : %d"), tryCount);
	for (auto& a : selectedAbilities)
	{
		UE_LOG(LogAbility, Log, TEXT("Selected Ability %d"), a.AbilityId);
	}

	return selectedAbilities;
}

int32 UAbilityManagerComponent::GetAbilityTotalTier(EAbilityType AbilityType)
{
	if (!AbilityTotalTier.Contains(AbilityType)) return 0;
	return AbilityTotalTier[AbilityType];
}

int32 UAbilityManagerComponent::GetAbilityTotalTierThreshold(EAbilityType AbilityType)
{
	if (!AbilityTotalTierThreshold.Contains(AbilityType)) return -1;
	return AbilityTotalTierThreshold[AbilityType];
}

void UAbilityManagerComponent::UpdateCumulativeProbabilityList()
{
	//누적합 배열 재구성
	CumulativeProbabilityList = { 0.0f }; //E_None
	TotalProbabilityValue = 0.0f;
	for (auto& probability : AbilityManagerInfo.ProbabilityInfoMap)
	{
		const float lastProbability = CumulativeProbabilityList.Last();
		TotalProbabilityValue += probability.Value;
		CumulativeProbabilityList.Add(probability.Value + lastProbability);
	}
}

bool UAbilityManagerComponent::TryUpdateCharacterStat(const FAbilityInfoStruct TargetAbilityInfo, bool bIsReset)
{
	//Validity 체크 (꺼져있는데 제거를 시도하거나, 켜져있는데 추가를 시도한다면?)
	if (GetIsAbilityActive(TargetAbilityInfo.AbilityId) != bIsReset) return false;
	
	FCharacterGameplayInfo& ownerInfo = OwnerCharacterRef.Get()->GetCharacterGameplayInfoRef();
	if (!ownerInfo.IsValid())
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryUpdateCharacterStat / ownerInfo is not valid"));
		return false;
	}

	ownerInfo.ResetAllAbilityStatInfo();

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
		ownerInfo.SetAbilityStatInfo(statType, ownerInfo.GetAbilityStatInfo(statType) + abilityValue.Variable);
		if (abilityValue.bIsContainProbability && ownerInfo.GetIsContainProbability(statType))
		{
			ownerInfo.SetProbabilityStatInfo(statType, ownerInfo.GetProbabilityStatInfo(statType) + abilityValue.ProbabilityValue);
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
	return AbilityManagerInfo.MaxAbilityCount;
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

int32 UAbilityManagerComponent::GetRandomAbilityIdxUsingGroupIdx(int32 SelectedGroupIdx)
{
	int32 resultAbilityIdx = -1; 

	//랜덤 adder로 티어결정
	if (CumulativeProbabilityList.Num() < 3)
	{
		const int32 randomAdder = UKismetMathLibrary::RandomInteger((int32)EAbilityTierType::E_Legendary);
		resultAbilityIdx = SelectedGroupIdx * 3 + randomAdder + 1;
	}
	//지정된 경우 
	else
	{
		float randomValue = UKismetMathLibrary::RandomFloatInRange(0.0f, TotalProbabilityValue);
		for (int32 idx = 1; idx < CumulativeProbabilityList.Num(); idx++)
		{
			if (randomValue >= CumulativeProbabilityList[idx - 1] && randomValue <= CumulativeProbabilityList[idx])
			{
				resultAbilityIdx = SelectedGroupIdx * 3 + idx;
				break;
			}
		}
	}
	return resultAbilityIdx;
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
	if (AbilityInfoTable == nullptr) return false;

	const TArray<FName> rowNameList = AbilityInfoTable->GetRowNames();
	AbilityManagerInfo.AbilityInfoList.Empty(); AbilityManagerInfo.AbilityPickedInfoList.Empty();
	AbilityManagerInfo.AbilityPickedInfoList.Add(false);
	AbilityManagerInfo.AbilityInfoList.Add(FAbilityInfoStruct());
	for (const FName& rowName : rowNameList)
	{
		FAbilityInfoStruct* abilityInfo = AbilityInfoTable->FindRow<FAbilityInfoStruct>(rowName, "");
		if (abilityInfo != nullptr)
		{
			AbilityManagerInfo.AbilityInfoList.Add(*abilityInfo);
			AbilityManagerInfo.AbilityPickedInfoList.Add(false);
			
			if (!AbilityTotalTierThreshold.Contains(abilityInfo->AbilityType))
				AbilityTotalTierThreshold.Add(abilityInfo->AbilityType, 0);
			if(abilityInfo->AbilityTier == EAbilityTierType::E_Legendary)
				AbilityTotalTierThreshold[abilityInfo->AbilityType] += (int32)abilityInfo->AbilityTier;
		}
	}
	return true;
}

bool UAbilityManagerComponent::CanPickAbility(int32 AbilityID)
{
	return !AbilityManagerInfo.AbilityPickedInfoList[(AbilityID - 1) / 3];
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
