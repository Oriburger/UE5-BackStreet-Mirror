// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerComponentBase.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SkillSystem/SkillBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"	
#include "../../Character/Component/WeaponComponentBase.h"

// Sets default values for this component's properties
USkillManagerComponentBase::USkillManagerComponentBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void USkillManagerComponentBase::BeginPlay()
{
	Super::BeginPlay();
	InitSkillManager();
}

void USkillManagerComponentBase::InitSkillManager()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	SkillStatTable = GameModeRef.Get()->SkillStatTable;
	checkf(IsValid(SkillStatTable), TEXT("Failed to get SkillStatDataTable"));
	OwnerCharacterRef->WeaponComponent->OnWeaponUpdated.AddDynamic(this, &USkillManagerComponentBase::InitSkillMap);
}

void USkillManagerComponentBase::InitSkillMap() {}

bool USkillManagerComponentBase::TrySkill(int32 SkillID)
{
	if (!IsSkillValid(SkillID)) return false;
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if (!IsValid(skillBase))
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::TrySkill(%d) : Skill Base Not Found"), SkillID);
		return false;
	}
	if (skillBase->SkillState.bIsBlocked) return false;
	if (skillBase->SkillStat.bIsLevelValid)
	{
		if (skillBase->SkillState.SkillLevel == 0) return false;
	}
	skillBase->ActivateSkill();
	OnSkillActivated.Broadcast(SkillID);
	return true;
}

bool USkillManagerComponentBase::AddSkill(int32 SkillID)
{
	if (IsSkillValid(SkillID)) return false;
	return true;
}

bool USkillManagerComponentBase::RemoveSkill(int32 SkillID)
{
	return true;
}

bool USkillManagerComponentBase::UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	if (!GetIsUpgradeTargetValid(SkillID, UpgradeTarget)) return false;
	if (!IsSkillValid(SkillID)) return false;
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if (!IsValid(skillBase)) return false;
	//skillBase->SkillState.SkillLevel = NewLevel;
	skillBase->SkillState.SkillUpgradeInfoMap[UpgradeTarget] = skillBase->SkillStat.LevelInfo[UpgradeTarget].SkillLevelInfoList[NewLevel];
	OnSkillUpdated.Broadcast();
	return true;
}

void USkillManagerComponentBase::ClearAllSkill() {}

FSkillStatStruct USkillManagerComponentBase::GetSkillInfo(int32 SkillID)
{
	FString rowName = FString::FromInt(SkillID);
	//checkf(IsValid(SkillStatTable), TEXT("SkillStatTable is not valid"));
	if (!SkillInfoCache.Contains(SkillID))
	{
		if (SkillStatTable)
		{
			FSkillStatStruct* skillStat = SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
			checkf(skillStat != nullptr, TEXT("SkillStat is not valid"));
			return SkillInfoCache.Add(SkillID, *skillStat);
		}
	}
	return SkillInfoCache[SkillID];
}

ESkillType USkillManagerComponentBase::GetSkillTypeInfo(int32 SkillID)
{
	return GetSkillInfo(SkillID).SkillWeaponStruct.SkillType;
}

ASkillBase* USkillManagerComponentBase::GetOwnSkillBase(int32 SkillID)
{
	return nullptr;
}

bool USkillManagerComponentBase::IsSkillValid(int32 SkillID)
{
	if (GetOwnSkillBase(SkillID) == nullptr) return false;
	return true;
}

bool USkillManagerComponentBase::GetIsSkillUpgradable(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	if (!GetIsUpgradeTargetValid(SkillID, UpgradeTarget)) return false;
	//최대 레벨보다 크면 업그레이드 불가
	if(GetOwnSkillStat(SkillID).LevelInfo[UpgradeTarget].SkillLevelInfoList.Num() - 1 < NewLevel) return false;
	//현재 레벨과 같거나 작은경우 업그레이드 불가
	if(GetOwnSkillState(SkillID).SkillUpgradeInfoMap[UpgradeTarget].CurrentLevel >= NewLevel) return false;
	return true;
}

FSkillStatStruct USkillManagerComponentBase::GetOwnSkillStat(int32 SkillID)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if(!IsValid(skillBase))
	{ 
		return GetSkillInfo(SkillID);
	}
	return skillBase->SkillStat;
}

FSkillStateStruct USkillManagerComponentBase::GetOwnSkillState(int32 SkillID)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if (!IsValid(skillBase))
	{
		return FSkillStateStruct();
	}
	return skillBase->SkillState;
}

FSkillUpgradeLevelInfo USkillManagerComponentBase::GetCurrentSkillLevelInfo(int32 SkillID, ESkillUpgradeType Target)
{
	if (!GetIsUpgradeTargetValid(SkillID, Target)) return FSkillUpgradeLevelInfo();
	return GetOwnSkillState(SkillID).SkillUpgradeInfoMap[Target];
}

FSkillUpgradeLevelInfo USkillManagerComponentBase::GetSkillUpgradeLevelInfo(int32 SkillID, ESkillUpgradeType Target, int32 TargetLevel)
{
	if (!GetIsUpgradeLevelValid(SkillID, Target, TargetLevel)) return FSkillUpgradeLevelInfo();
	return GetOwnSkillStat(SkillID).LevelInfo[Target].SkillLevelInfoList[TargetLevel];
}

bool USkillManagerComponentBase::GetIsUpgradeTargetValid(int32 SkillID, ESkillUpgradeType UpgradeTarget)
{
	if (!GetOwnSkillStat(SkillID).LevelInfo.Contains(UpgradeTarget)
		|| !GetOwnSkillState(SkillID).SkillUpgradeInfoMap.Contains(UpgradeTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("USkillManagerComponentBase::GetIsUpgradeTargetValid(%d, %d) is not valid"), SkillID, (int32)UpgradeTarget);
		return false;
	}
	return true;
}

bool USkillManagerComponentBase::GetIsUpgradeLevelValid(int32 SkillID, ESkillUpgradeType UpgradeTarget, int32 TargetLevel)
{
	if (!GetIsUpgradeTargetValid(SkillID, UpgradeTarget)) return false;
	return GetOwnSkillStat(SkillID).LevelInfo[UpgradeTarget].SkillLevelInfoList.IsValidIndex(TargetLevel);
}
