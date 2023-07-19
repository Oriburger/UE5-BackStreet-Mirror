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
	UE_LOG(LogTemp, Warning, TEXT("Initialize Ability Manager Success"));
	OwnerCharacterRef = NewCharacter;

	//초기화 시점에 진행
	UDataTable* abilityInfoTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Character/MainCharacter/Data/D_AbilityInfoDataTable.D_AbilityInfoDataTable'"));
	if (!InitAbilityInfoListFromTable(abilityInfoTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("UAbilityManagerBase::InitAbilityManager) DataTable is not found!"));
	}
}

bool UAbilityManagerBase::TryAddNewAbility(const ECharacterAbilityType NewAbilityType)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();

	if (GetIsAbilityActive(NewAbilityType)) return false;
	if (ActiveAbilityInfoList.Num() >= MaxAbilityCount) return false;

	FAbilityInfoStruct newAbilityInfo = GetAbilityInfo(NewAbilityType);
	if (newAbilityInfo.AbilityId == -1) return false;

	if (newAbilityInfo.bIsRepetitive)
	{
		const float variable = newAbilityInfo.Variable.Num() > 0 ? newAbilityInfo.Variable[0] : 1.0f;
		newAbilityInfo.TimerDelegate.BindUFunction(OwnerCharacterRef.Get(), newAbilityInfo.FuncName, variable, true, (uint8)NewAbilityType);
		OwnerCharacterRef.Get()->GetWorldTimerManager().SetTimer(newAbilityInfo.TimerHandle, newAbilityInfo.TimerDelegate, 1.0f, true);
	}
	TryUpdateCharacterStat(newAbilityInfo, false);
	ActiveAbilityInfoList.Add(newAbilityInfo);
	AbilityAddDelegate.Broadcast(newAbilityInfo);

	return true;
}

bool UAbilityManagerBase::TryRemoveAbility(ECharacterAbilityType TargetAbilityType)
{
	if (!OwnerCharacterRef.IsValid()) return false;
	if (!GetIsAbilityActive(TargetAbilityType)) return false;
	if (ActiveAbilityInfoList.Num() == 0) return false;

	FAbilityInfoStruct targetAbilityInfo = GetAbilityInfo(TargetAbilityType);
	targetAbilityInfo.AbilityId = (uint8)TargetAbilityType;
	if (targetAbilityInfo.AbilityId == -1) return false;
	
	for (int idx = 0; idx < ActiveAbilityInfoList.Num(); idx++)
	{
		FAbilityInfoStruct& abilityInfo = ActiveAbilityInfoList[idx];
		if (abilityInfo.AbilityId == (uint8)TargetAbilityType)
		{
			TryUpdateCharacterStat(abilityInfo, true);
			ActiveAbilityInfoList.RemoveAt(idx);
			if (abilityInfo.bIsRepetitive)
			{
				OwnerCharacterRef.Get()->GetWorldTimerManager().ClearTimer(abilityInfo.TimerHandle);
			}
			AbilityRemoveDelegate.Broadcast((uint8)TargetAbilityType);
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
	if (GetIsAbilityActive((ECharacterAbilityType)TargetAbilityInfo.AbilityId) != bIsReset) return false;
	
	FCharacterStatStruct characterStat = OwnerCharacterRef.Get()->GetCharacterStat();
	FCharacterStateStruct characterState = OwnerCharacterRef.Get()->GetCharacterState();

	for (int statIdx = 0; statIdx < TargetAbilityInfo.TargetStatName.Num(); statIdx++)
	{
		FName targetStatName = TargetAbilityInfo.TargetStatName[statIdx];
		float targetVariable = TargetAbilityInfo.Variable[FMath::Min(TargetAbilityInfo.Variable.Num() - 1, statIdx)];
		if (bIsReset) targetVariable = 1/targetVariable; //초기화 시 1보다 낮은 값으로 곱함 1.25 vs 0.25

		if (targetStatName == FName("MaxHP"))
		{
			characterStat.CharacterMaxHP *= targetVariable;
			characterState.CharacterCurrHP = FMath::Min(characterStat.CharacterMaxHP, characterState.CharacterCurrHP);
		}
		//else if (targetStatName == FName("CurrHP"))
		//	characterState.CharacterCurrHP = characterStat.CharacterMaxHP;
		else if (targetStatName == FName("Attack"))
			characterStat.CharacterAtkMultiplier *= targetVariable;
		else if (targetStatName == FName("Defense"))
			characterStat.CharacterDefense *= targetVariable;
		else if (targetStatName == FName("MoveSpeed"))
			characterStat.CharacterMoveSpeed *= targetVariable;
		else if (targetStatName == FName("AttackSpeed"))
			characterStat.CharacterAtkSpeed *= targetVariable;
		else if (targetStatName == FName("MaxProjectileCount"))
			characterStat.MaxProjectileCount *= targetVariable;
	}
	OwnerCharacterRef.Get()->UpdateCharacterStat(characterStat);
	OwnerCharacterRef.Get()->UpdateCharacterState(characterState);

	return true;
}

bool UAbilityManagerBase::GetIsAbilityActive(const ECharacterAbilityType TargetAbilityType) const
{
	for (const FAbilityInfoStruct& abilityInfo : ActiveAbilityInfoList)
	{
		if (abilityInfo.AbilityId == (uint8)TargetAbilityType)
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

FAbilityInfoStruct UAbilityManagerBase::GetAbilityInfo(const ECharacterAbilityType AbilityType)
{
	if(!AbilityInfoList.IsValidIndex((uint8)AbilityType)) return FAbilityInfoStruct();
	return AbilityInfoList[(uint8)AbilityType];
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
