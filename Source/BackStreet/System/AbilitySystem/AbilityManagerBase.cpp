// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityManagerBase.h"
#include "../../Character/CharacterBase.h"
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

void UAbilityManagerComponent::InitAbilityManager(ACharacterBase* NewCharacter)
{
	if (!IsValid(NewCharacter)) return;
	//UE_LOG(LogTemp, Warning, TEXT("Initialize Ability Manager Success"));
	OwnerCharacterRef = NewCharacter;

	//초기화 시점에 진행
	if (!InitAbilityInfoListFromTable())
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

	//이전 단계의 어빌리티가 있다면 제거부터 함 // not working 250107 
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
	AbilityPickedInfoList[AbilityID / 3] = true;
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
		if (!ownerInfo.StatGroupList.IsValidIndex((int32)statType))
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

void UAbilityManagerComponent::UpdateProbilityValue(EAbilityTierType TierType, float NewProbability)
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

TArray<FAbilityInfoStruct> UAbilityManagerComponent::GetRandomAbilityInfoList(int32 Count, TArray<EAbilityType> TypeList)
{
	if (AbilityPickedInfoList.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList, AbilityPickedInfoList is empty"));
		return {};
	}
	
	TArray<FAbilityInfoStruct> selectedAbilities;
	TArray<int32> candidateGroups, pickedGroupIdx;
	
	//무한 루프 방지 코드
	int32 tryCount = 0; 
	const int32 tryCountThreshold = Count * 100;

	// TypeList와 단계 확인을 통과한 후보들 필터링
	for (int32 groupIdx = 0; groupIdx < AbilityInfoList.Num() / 3; ++groupIdx)
	{
		const FAbilityInfoStruct& abilityInfo = AbilityInfoList[groupIdx * 3 + 1];
		if (TypeList.Contains(abilityInfo.AbilityType) && CanPickAbility(abilityInfo.AbilityId))
		{
			candidateGroups.Add(groupIdx);
		}
	}
	while (selectedAbilities.Num() < Count && selectedAbilities.Num() < candidateGroups.Num())
	{
		tryCount += 1; 
		if (tryCount >= tryCountThreshold) break;

		const int32 randomCandidateIdx = UKismetMathLibrary::RandomInteger(candidateGroups.Num());
		const int32 selectedGroupIdx = candidateGroups[randomCandidateIdx];
		if (pickedGroupIdx.Contains(selectedGroupIdx))
		{
			continue; 
		}

		//확률 프로퍼티가 지정되어있지 않을 경우
		if (CumulativeProbabilityList.Num() < 3)
		{
			//랜덤 adder로 티어결정
			const int32 randomAdder = UKismetMathLibrary::RandomInteger((int32)EAbilityTierType::E_Legendary);
			const int32 resultAbilityIdx = selectedGroupIdx * 3 + randomAdder + 1;

			if (AbilityInfoList.IsValidIndex(resultAbilityIdx))
			{
				selectedAbilities.Add(AbilityInfoList[resultAbilityIdx]);
				pickedGroupIdx.Add(selectedGroupIdx);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList(%d) failed - selected Group %d / random Adder %d / resultAbilityIdx %d"), selectedGroupIdx, randomAdder, resultAbilityIdx);
			}
		}
		//지정된 경우 
		else
		{
			float randomValue = UKismetMathLibrary::RandomFloatInRange(0.0f, TotalProbabilityValue);
			int32 resultAbilityIdx = -1;

			for (int32 idx = 1; idx < CumulativeProbabilityList.Num(); idx++)
			{
				if (randomValue >= CumulativeProbabilityList[idx - 1] && randomValue <= CumulativeProbabilityList[idx])
				{
					resultAbilityIdx = selectedGroupIdx * 3 + idx; 
					pickedGroupIdx.Add(selectedGroupIdx);
					break;
				}
			}
			if (resultAbilityIdx != -1 && AbilityInfoList.IsValidIndex(resultAbilityIdx))
			{
				selectedAbilities.Add(AbilityInfoList[resultAbilityIdx]);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("UAbilityManagerComponent::GetRandomAbilityInfoList(%d) error with cumulativeprobList - selected Group %d / random Value %.2lf / resultAbilityIdx %d"), selectedGroupIdx, randomValue, resultAbilityIdx);
			}
		}
	}

	return selectedAbilities;
}

void UAbilityManagerComponent::UpdateCumulativeProbabilityList()
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

		if (!ownerInfo.StatGroupList.IsValidIndex((int32)statType))
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

bool UAbilityManagerComponent::InitAbilityInfoListFromTable()
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
	return !AbilityPickedInfoList[AbilityID / 3];
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
