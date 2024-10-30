// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityManagerBase.h"
#include "../../Character/CharacterBase.h"

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
	UDataTable* abilityInfoTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Character/MainCharacter/Data/D_AbilityInfoDataTable.D_AbilityInfoDataTable'"));
	if (!InitAbilityInfoListFromTable(abilityInfoTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("UAbilityManagerComponent::InitAbilityManager) DataTable is not found!"));
	}
}

bool UAbilityManagerComponent::TryAddNewAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	
	if (GetIsAbilityActive(AbilityID)) return false;
	if (ActiveAbilityInfoList.Num() >= MaxAbilityCount) return false;
	FAbilityInfoStruct newAbilityInfo = GetAbilityInfo(AbilityID);
	if (newAbilityInfo.bIsRepetitive)
	{	
		float variable = newAbilityInfo.VariableInfo.Num() > 0 ? newAbilityInfo.VariableInfo[0].Variable : 1.0f;
		newAbilityInfo.TimerDelegate.BindUFunction(OwnerCharacterRef.Get(), newAbilityInfo.FuncName, variable, true, AbilityID);
		OwnerCharacterRef.Get()->GetWorldTimerManager().SetTimer(newAbilityInfo.TimerHandle, newAbilityInfo.TimerDelegate, 1.0f, true);
	}
	TryUpdateCharacterStat(newAbilityInfo, false);
	ActiveAbilityInfoList.Add(newAbilityInfo);
	AbilityUpdateDelegate.Broadcast(ActiveAbilityInfoList);

	return true;
}

bool UAbilityManagerComponent::TryRemoveAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (!GetIsAbilityActive(AbilityID)) return false;
	if (ActiveAbilityInfoList.Num() == 0) return false;

	FAbilityInfoStruct targetAbilityInfo = GetAbilityInfo(AbilityID);
	if (targetAbilityInfo.AbilityId <= 0) return false;
	
	for (int idx = 0; idx < ActiveAbilityInfoList.Num(); idx++)
	{
		FAbilityInfoStruct& abilityInfo = ActiveAbilityInfoList[idx];
		if (abilityInfo.AbilityId == targetAbilityInfo.AbilityId)
		{
			TryUpdateCharacterStat(abilityInfo, true);
			ActiveAbilityInfoList.RemoveAt(idx);
			if (abilityInfo.bIsRepetitive)
			{
				OwnerCharacterRef.Get()->GetWorldTimerManager().ClearTimer(abilityInfo.TimerHandle);
			}
			AbilityUpdateDelegate.Broadcast(ActiveAbilityInfoList);
			return true;
		}
	}
	return false;
}

void UAbilityManagerComponent::ClearAllAbility()
{
	ActiveAbilityInfoList.Empty();
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

	for (int statIdx = 0; statIdx < TargetAbilityInfo.AbilityTypeList.Num(); statIdx++)
	{
		ECharacterAbilityType targetType = TargetAbilityInfo.AbilityTypeList[FMath::Min(TargetAbilityInfo.AbilityTypeList.Num() - 1, statIdx)];
		FAbilityValueInfoStruct targetVariableInfo = TargetAbilityInfo.VariableInfo[FMath::Min(TargetAbilityInfo.VariableInfo.Num() - 1, statIdx)];
		
		float targetVariable = targetVariableInfo.Variable;
		bool bIsPercentage = targetVariableInfo.bIsPercentage;

		if (bIsReset) targetVariable = -1 * targetVariable; //if remove ability, substract the original value

		switch (targetType)
		{
		case ECharacterAbilityType::E_None:
		case ECharacterAbilityType::E_AutoHeal:
			break;
		case ECharacterAbilityType::E_MaxHP:
			ownerInfo.SetAbilityStatInfo(ECharacterStatType::E_MaxHealth, targetVariable);
			break;
		case ECharacterAbilityType::E_AttackUp:
			ownerInfo.SetAbilityStatInfo(ECharacterStatType::E_NormalPower, targetVariable);
			break;
		case ECharacterAbilityType::E_DefenseUp:
			ownerInfo.SetAbilityStatInfo(ECharacterStatType::E_Defense, targetVariable);
			break;
		case ECharacterAbilityType::E_MoveSpeedUp:
			ownerInfo.SetAbilityStatInfo(ECharacterStatType::E_MoveSpeed, targetVariable);
			break;
		case ECharacterAbilityType::E_AtkSpeedUp:
			//ownerInfo.SetAbilityStatInfo(ECharacterStatType::, targetVariable);
			break;
		case ECharacterAbilityType::E_MultipleShot:
			//characterStat.ProjectileCountPerAttack += targetVariable;
			break;
		case ECharacterAbilityType::E_LargeWishList:
			ownerInfo.MaxKeepingSkillCount += targetVariable;
			break;
		case ECharacterAbilityType::E_ExtraTime:
			ownerInfo.ExtraStageTime += targetVariable;
			break;
		case ECharacterAbilityType::E_LuckyMaterial:
			ownerInfo.ExtraPercentageUnivMaterial += targetVariable;
			break;
		case ECharacterAbilityType::E_InfiniteSkillMaterial:
			ownerInfo.bInfiniteSkillMaterial = (bool)targetVariable;
			break;
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

FAbilityInfoStruct UAbilityManagerComponent::GetAbilityInfo(int32 AbilityID)
{
	if(!AbilityInfoList.IsValidIndex(AbilityID)) return FAbilityInfoStruct();
	return AbilityInfoList[AbilityID];
}

bool UAbilityManagerComponent::InitAbilityInfoListFromTable(const UDataTable* AbilityInfoTable)
{
	if (AbilityInfoTable == nullptr) return false;

	const TArray<FName> rowNameList = AbilityInfoTable->GetRowNames();
	AbilityInfoList.Empty();
	AbilityInfoList.Add(FAbilityInfoStruct());
	for (const FName& rowName : rowNameList)
	{
		FAbilityInfoStruct* abilityInfo = AbilityInfoTable->FindRow<FAbilityInfoStruct>(rowName, "");
		if (abilityInfo != nullptr)
		{
			AbilityInfoList.Add(*abilityInfo);
		}
	}
	return true;
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
