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

bool USkillManagerComponentBase::TryAddSkill(int32 NewSkillID)
{
	bool bIsInInventory = false;
	FSkillInfo skillInfo = GetSkillInfoData(NewSkillID, bIsInInventory);
	if (!skillInfo.IsValid() || bIsInInventory)
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::TryAddSkill %d failed - Reason %d %d"), NewSkillID, !skillInfo.IsValid(), bIsInInventory);
		return false;
	}
	SkillInventory.Add(NewSkillID, skillInfo);
	return true;
}

bool USkillManagerComponentBase::TryRemoveSkill(int32 TargetSkillID)
{
	bool bIsInInventory = false;
	FSkillInfo skillInfo = GetSkillInfoData(TargetSkillID, bIsInInventory);
	if (!skillInfo.IsValid() || !bIsInInventory)
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::TryRemoveSkill %d failed"), TargetSkillID);
		return false;
	}
	SkillInventory.Remove(TargetSkillID);
	return true;
}

bool USkillManagerComponentBase::TryActivateSkill(int32 TargetSkillID)
{
	bool bIsInInventory = false;
	FSkillInfo skillInfo = GetSkillInfoData(TargetSkillID, bIsInInventory);
	if (!skillInfo.IsValid() || !bIsInInventory || !OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::TryActivateSkill %d failed - Reason %d %d %d"), TargetSkillID, !skillInfo.IsValid(), !bIsInInventory, !OwnerCharacterRef.IsValid());
		return false;
	}
	if (!skillInfo.bContainPrevAction)
	{
		OwnerCharacterRef.Get()->PlayAnimMontage(skillInfo.SkillAnimation);
		return true;
	}
	else if(skillInfo.PrevActionClass != nullptr)
	{
		ASkillBase* prevAction = Cast<ASkillBase>(GetWorld()->SpawnActor(skillInfo.PrevActionClass));
		prevAction->InitSkill(FSkillStatStruct(), this); //제거 필요
		prevAction->ActivateSkill();
	}
	return true;
}

FSkillInfo USkillManagerComponentBase::GetSkillInfoData(int32 TargetSkillID, bool& bIsInInventory)
{
	if (SkillInventory.Contains(TargetSkillID))
	{
		bIsInInventory = true;
		return SkillInventory[TargetSkillID];
	}

	//Enemy doesn't use the inventory data. 
	bIsInInventory = OwnerCharacterRef.IsValid() ? OwnerCharacterRef.Get()->ActorHasTag("Enemy") : false;
	FString rowName = FString::FromInt(TargetSkillID);
	if (!SkillInfoCache.Contains(TargetSkillID))
	{
		if (SkillStatTable)
		{
			FSkillInfo* skillInfo = SkillInfoTable->FindRow<FSkillInfo>(FName(rowName), rowName);
			if (skillInfo == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::GetSkillInfoData %d is not found"), TargetSkillID);
				return FSkillInfo();
			}
			return SkillInfoCacheMap.Add(TargetSkillID, *skillInfo);
		}
	}
	return SkillInfoCacheMap[TargetSkillID];
}

//================================================================
//================================================================
//================================================================

void USkillManagerComponentBase::InitSkillManager()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	SkillStatTable = GameModeRef.Get()->SkillStatTable;
	checkf(IsValid(SkillStatTable), TEXT("Failed to get SkillStatDataTable"));
	OwnerCharacterRef->WeaponComponent->OnMainWeaponUpdated.AddDynamic(this, &USkillManagerComponentBase::InitSkillMap);
	UE_LOG(LogTemp, Warning, TEXT("USkillManagerComponentBase::InitSkillManager()"));
}

void USkillManagerComponentBase::InitSkillMap() 
{
	UE_LOG(LogTemp, Warning, TEXT("InitSkillMap()"));
}

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
		//if (skillBase->SkillState.SkillLevel == 0) return false;
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

void USkillManagerComponentBase::ClearAllSkill() 
{
	UE_LOG(LogTemp, Warning, TEXT("USkillManagerComponentBase::ClearAllSkill() "));
}

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
