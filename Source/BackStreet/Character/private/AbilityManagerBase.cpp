// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/AbilityManagerBase.h"
#include "../../Character/public/CharacterBase.h"

// Sets default values
UAbilityManagerBase::UAbilityManagerBase()
{
	
}

void UAbilityManagerBase::InitAbilityManager(ACharacterBase* NewCharacter)
{
	if (!IsValid(NewCharacter)) return;
	//UE_LOG(LogTemp, Warning, TEXT("Initialize Ability Manager Success"));
	OwnerCharacterRef = NewCharacter;

	//초기화 시점에 진행
	UDataTable* abilityInfoTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Character/MainCharacter/Data/D_AbilityInfoDataTable.D_AbilityInfoDataTable'"));
	if (!InitAbilityInfoListFromTable(abilityInfoTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("UAbilityManagerBase::InitAbilityManager) DataTable is not found!"));
	}
}

bool UAbilityManagerBase::TryAddNewAbility(int32 AbilityID)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();

	if (GetIsAbilityActive(AbilityID)) return false;
	if (ActiveAbilityInfoList.Num() >= MaxAbilityCount) return false;
	FAbilityInfoStruct newAbilityInfo = GetAbilityInfo(AbilityID);
	if (newAbilityInfo.bIsRepetitive)
	{
		const float variable = newAbilityInfo.Variable.Num() > 0 ? newAbilityInfo.Variable[0] : 1.0f;
		newAbilityInfo.TimerDelegate.BindUFunction(OwnerCharacterRef.Get(), newAbilityInfo.FuncName, variable, true, AbilityID);
		OwnerCharacterRef.Get()->GetWorldTimerManager().SetTimer(newAbilityInfo.TimerHandle, newAbilityInfo.TimerDelegate, 1.0f, true);
	}
	TryUpdateCharacterStat(newAbilityInfo, false);
	ActiveAbilityInfoList.Add(newAbilityInfo);
	AbilityUpdateDelegate.Broadcast(ActiveAbilityInfoList);

	return true;
}

bool UAbilityManagerBase::TryRemoveAbility(int32 AbilityID)
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

void UAbilityManagerBase::ClearAllAbility()
{
	ActiveAbilityInfoList.Empty();
}

bool UAbilityManagerBase::TryUpdateCharacterStat(const FAbilityInfoStruct TargetAbilityInfo, bool bIsReset)
{
	//Validity 체크 (꺼져있는데 제거를 시도하거나, 켜져있는데 추가를 시도한다면?)
	if (GetIsAbilityActive(TargetAbilityInfo.AbilityId) != bIsReset) return false;
	
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();
	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();

	for (int statIdx = 0; statIdx < TargetAbilityInfo.AbilityTypeList.Num(); statIdx++)
	{
		ECharacterAbilityType targetType = TargetAbilityInfo.AbilityTypeList[FMath::Min(TargetAbilityInfo.AbilityTypeList.Num() - 1, statIdx)];
		float targetVariable = TargetAbilityInfo.Variable[FMath::Min(TargetAbilityInfo.Variable.Num() - 1, statIdx)];
		if (bIsReset) targetVariable = -1 * targetVariable; //if remove ability, substract the original value

		switch (targetType)
		{
		case ECharacterAbilityType::E_None:
		case ECharacterAbilityType::E_AutoHeal:
			break;
		case ECharacterAbilityType::E_DefaultHP:
			characterStat.AbilityHP += targetVariable;
			characterState.CurrentHP += targetVariable;
			break;
		case ECharacterAbilityType::E_AttackUp:
			characterStat.AbilityAttack += targetVariable;
			break;
		case ECharacterAbilityType::E_DefenseUp:
			characterStat.DefaultDefense += targetVariable;
			break;
		case ECharacterAbilityType::E_MoveSpeedUp:
			characterStat.DefaultMoveSpeed += targetVariable;
			break;
		case ECharacterAbilityType::E_AtkSpeedUp:
			characterStat.DefaultAttackSpeed += targetVariable;
			break;
		case ECharacterAbilityType::E_MultipleShot:
			characterStat.ProjectileCountPerAttack += targetVariable;
			break;
		}	
	}
	OwnerCharacterRef.Get()->UpdateCharacterStatAndState(characterStat, characterState);

	return true;
}

bool UAbilityManagerBase::GetIsAbilityActive(int32 AbilityID) const
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

int32 UAbilityManagerBase::GetMaxAbilityCount() const
{
	return MaxAbilityCount; 
}

FAbilityInfoStruct UAbilityManagerBase::GetAbilityInfo(int32 AbilityID)
{
	if(!AbilityInfoList.IsValidIndex(AbilityID)) return FAbilityInfoStruct();
	return AbilityInfoList[AbilityID];
}

bool UAbilityManagerBase::InitAbilityInfoListFromTable(const UDataTable* AbilityInfoTable)
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

TArray<ECharacterAbilityType> UAbilityManagerBase::GetActiveAbilityList() const
{
	TArray<ECharacterAbilityType> returnActiveAbility;
	for (const FAbilityInfoStruct& abilityInfo : ActiveAbilityInfoList)
	{
		returnActiveAbility.Add((const ECharacterAbilityType)abilityInfo.AbilityId);
	}
	return returnActiveAbility;
}

TArray<FAbilityInfoStruct> UAbilityManagerBase::GetActiveAbilityInfoList() const
{
	return ActiveAbilityInfoList;
}
