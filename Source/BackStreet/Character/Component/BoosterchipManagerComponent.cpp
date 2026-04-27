// Fill out your copyright notice in the Description page of Project Settings.


#include "BoosterchipManagerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Character/CharacterBase.h"

// Sets default values
UBoosterchipManagerComponent::UBoosterchipManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UBoosterchipManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateCumulativeProbabilityList();
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	InitBoosterchipManager(OwnerCharacterRef.Get(), FBoosterchipManagerInfo());
}

void UBoosterchipManagerComponent::InitBoosterchipManager(ACharacterBase* NewCharacter, FBoosterchipManagerInfo InfoOverride)
{
	UE_LOG(LogAbility, Log, TEXT("UBoosterchipManagerComponent::InitBoosterchipManager"));
	InitAbilityManager(NewCharacter, InfoOverride.AbilityManagerInfo);
	if (!InfoOverride.AbilityPickedInfoList.IsEmpty())
	{
		BoosterchipManagerInfo = InfoOverride;
	}
}

bool UBoosterchipManagerComponent::TryAddNewAbility(int32 AbilityID)
{
	if (!Super::TryAddNewAbility(AbilityID)) return false;
	BoosterchipManagerInfo.AbilityPickedInfoList[(AbilityID - 1) / 3] = true;
	return true;
}

bool UBoosterchipManagerComponent::TryRemoveAbility(int32 AbilityID)
{
	return Super::TryRemoveAbility(AbilityID);
}

void UBoosterchipManagerComponent::ClearAllAbility()
{
	AbilityManagerInfo.ActiveAbilityInfoList.Empty();
}

bool UBoosterchipManagerComponent::TryUpgradeAbility(int32 AbilityID)
{
	//3개의 티어이기 때문에, 3으로 나누어 떨어지는 AbilityID는 업그레이드 불가능
	if (AbilityID % 3 == 0)
	{
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryUpgradeAbility / AbilityID %d is not valid for upgrade"), AbilityID);
		return false; // 업그레이드 불가능한 어빌리티
	}

	bool result = TryRemoveAbility(AbilityID);
	if (result)
	{
		result = TryAddNewAbility(AbilityID + 1);
		if(!result)
		{
			UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::TryUpgradeAbility / Failed to add upgraded AbilityID %d"), AbilityID + 1);
			//업그레이드 실패 시, 기존 어빌리티 복구
			TryAddNewAbility(AbilityID);
		}
	}
	return result; // AbilityID가 ActiveAbilityInfoList에 존재하지 않는 경우
}

void UBoosterchipManagerComponent::UpdateProbilityValue(EAbilityTierType TierType, float NewProbability)
{
	if (!ProbabilityInfoMap.Contains(TierType))
	{
		ProbabilityInfoMap.Add(TierType, NewProbability);
	}
	else
	{
		ProbabilityInfoMap[TierType] = NewProbability;
	}
	UpdateCumulativeProbabilityList();
}

int32 UBoosterchipManagerComponent::GetAbilityUpgradeCost(EAbilityTierType TargetTier)
{
	if (!UpgradeCostInfoMap.Contains(TargetTier)) return -1;
	return UpgradeCostInfoMap[TargetTier];
}

FBoosterchipManagerInfo UBoosterchipManagerComponent::GetBoosterchipManagerInfo()
{
	BoosterchipManagerInfo.AbilityManagerInfo = AbilityManagerInfo;
	return BoosterchipManagerInfo;
}

TArray<FAbilityInfoStruct> UBoosterchipManagerComponent::GetRandomAbilityInfoList(int32 Count, TArray<EAbilityType> TypeList)
{
	if (BoosterchipManagerInfo.AbilityPickedInfoList.IsEmpty())
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
		UE_LOG(LogAbility, Error, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList, candidateGroups is empty, abilityinfolist num : %d"), AbilityManagerInfo.AbilityInfoList.Num());
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

int32 UBoosterchipManagerComponent::GetRandomAbilityIdxUsingGroupIdx(int32 SelectedGroupIdx)
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

bool UBoosterchipManagerComponent::InitAbilityInfoListFromTable()
{
	if (!Super::InitAbilityInfoListFromTable())
	{
		UE_LOG(LogAbility, Error, TEXT("UBoosterchipManagerComponent::InitAbilityInfoListFromTable / Super::InitAbilityInfoListFromTable failed"));
		return false;
	}

	const TArray<FName> rowNameList = AbilityDataTable->GetRowNames();
	BoosterchipManagerInfo.AbilityPickedInfoList.Empty();
	BoosterchipManagerInfo.AbilityPickedInfoList.Add(false);

	for (const FName& rowName : rowNameList)
	{
		FAbilityInfoStruct* abilityInfo = AbilityDataTable->FindRow<FAbilityInfoStruct>(rowName, "");
		if (abilityInfo != nullptr)
		{
			BoosterchipManagerInfo.AbilityPickedInfoList.Add(false);

			if (!AbilityTotalTierThreshold.Contains(abilityInfo->AbilityType))
				AbilityTotalTierThreshold.Add(abilityInfo->AbilityType, 0);
			if (abilityInfo->AbilityTier == EAbilityTierType::E_Legendary)
				AbilityTotalTierThreshold[abilityInfo->AbilityType] += (int32)abilityInfo->AbilityTier;
		}
	}
	return true;
}

bool UBoosterchipManagerComponent::CanPickAbility(int32 AbilityID)
{
	return !BoosterchipManagerInfo.AbilityPickedInfoList[(AbilityID - 1) / 3];
}

void UBoosterchipManagerComponent::UpdateCumulativeProbabilityList()
{
	//누적합 배열 재구성
	CumulativeProbabilityList = { 0.0f }; //E_None
	TotalProbabilityValue = 0.0f;
	for (auto& probability : ProbabilityInfoMap)
	{
		const float lastProbability = CumulativeProbabilityList.Last();
		TotalProbabilityValue += probability.Value;
		CumulativeProbabilityList.Add(probability.Value + lastProbability);
	}
}

int32 UBoosterchipManagerComponent::GetAbilityTotalTier(EAbilityType AbilityType)
{
	if (!AbilityManagerInfo.AbilityTotalTier.Contains(AbilityType)) return 0;
	return AbilityManagerInfo.AbilityTotalTier[AbilityType];
}

int32 UBoosterchipManagerComponent::GetAbilityTotalTierThreshold(EAbilityType AbilityType)
{
	if (!AbilityTotalTierThreshold.Contains(AbilityType)) return -1;
	return AbilityTotalTierThreshold[AbilityType];
}