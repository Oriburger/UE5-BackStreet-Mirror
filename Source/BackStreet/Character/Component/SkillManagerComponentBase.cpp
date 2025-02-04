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
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	OwnerCharacterRef->WeaponComponent->OnMainWeaponUpdated.AddDynamic(this, &USkillManagerComponentBase::ClearAllSkill);

	//Init Skill Map
	InitSkillInfoCache();
}

void USkillManagerComponentBase::InitSkillInfoCache()
{
	if (!SkillInfoCacheMap.IsEmpty()) return;
	if (SkillInfoTable != nullptr)
	{
		TArray<FName> rowNameList = SkillInfoTable->GetRowNames();
		for (auto& row : rowNameList)
		{
			FString contextString = "";
			FSkillInfo* skillInfo = SkillInfoTable->FindRow<FSkillInfo>(row, contextString);
			if (skillInfo == nullptr) continue;
			SkillInfoCacheMap.Add(skillInfo->SkillID, *skillInfo);

			if (skillInfo->bIsPlayerSkill)
			{
				PlayerSkillIDList.Add(skillInfo->SkillID);
			}
		}
	}
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

	if (skillInfo.CoolTimeValue > 0.0f && !CoolTimerHandleMap.Contains(NewSkillID))
	{
		CoolTimerHandleMap.Add(NewSkillID, FTimerHandle());
	}

	TArray<FSkillInfo> skillInfoList;
	SkillInventory.GenerateValueArray(skillInfoList);
	OnSkillInventoryUpdated.Broadcast(skillInfoList);
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
	if (skillInfo.CoolTimeValue > 0.0f && CoolTimerHandleMap.Contains(TargetSkillID))
	{
		CoolTimerHandleMap[TargetSkillID].Invalidate();
		GetWorld()->GetTimerManager().ClearTimer(CoolTimerHandleMap[TargetSkillID]);
		CoolTimerHandleMap.Remove(TargetSkillID);
	}
	TArray<FSkillInfo> skillInfoList;
	SkillInventory.GenerateValueArray(skillInfoList);
	OnSkillInventoryUpdated.Broadcast(skillInfoList);
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
	
	if (PrevSkillInfo.PrevActionRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("USkilllManagercomponentBase::TryActivateSkill prevSkill is not Destroyed - Reason %d"), !PrevSkillInfo.PrevActionRef.IsValid());
		PrevSkillInfo.PrevActionRef.Get()->Destroy();
	}

	if (!skillInfo.bContainPrevAction)
	{
		OwnerCharacterRef.Get()->PlayAnimMontage(skillInfo.SkillAnimation);
		PrevSkillInfo = skillInfo;
	}
	else if (skillInfo.PrevActionClass != nullptr)
	{
		ASkillBase* prevAction = Cast<ASkillBase>(GetWorld()->SpawnActor(skillInfo.PrevActionClass));
		prevAction->InitSkill(FSkillStatStruct(), this, skillInfo); //제거 필요
		prevAction->ActivateSkill();
		PrevSkillInfo = skillInfo;
	}
	else return false;

	OnSkillActivated.Broadcast(skillInfo);

	if (skillInfo.CoolTimeValue > 0.0f && CoolTimerHandleMap.Contains(TargetSkillID))
	{
		FTimerDelegate timerDelegate;
		timerDelegate.BindUFunction(this, FName("UpdateSkillValidity"), TargetSkillID, true);
		UpdateSkillValidity(TargetSkillID, false);
		const float coolTimeAbilityValue = OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_SkillCoolTime) - 1.0f;
		const float totalCoolTimeValue = FMath::Clamp(skillInfo.CoolTimeValue - skillInfo.CoolTimeValue * coolTimeAbilityValue, 0.1f, 60.0f);
		GetWorld()->GetTimerManager().SetTimer(CoolTimerHandleMap[TargetSkillID], timerDelegate, totalCoolTimeValue, false);
	}

	return true;
}

void USkillManagerComponentBase::DeactivateCurrentSkill()
{
	if (PrevSkillInfo.PrevActionRef.IsValid())
	{
		ASkillBase* prevSkillBase = Cast<ASkillBase>(PrevSkillInfo.PrevActionRef);
		prevSkillBase->DeactivateSkill();
		prevSkillBase->Destroy();
	}
	PrevSkillInfo.IsValid();
	OnSkillDeactivated.Broadcast(PrevSkillInfo);

	PrevSkillInfo = FSkillInfo();
}

TArray<int32> USkillManagerComponentBase::GetCraftableIDList()
{
	TArray<int32> resultList;

	for (int32& id : PlayerSkillIDList)
	{
		if (!SkillInventory.Contains(id))
		{
			resultList.Add(id);
		}
	}
	return resultList;
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
		if (SkillInfoTable == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::GetSkillInfoData SkillInfoTable is not found"));
			return FSkillInfo();
		}
		else
		{
			FSkillInfo* skillInfo = SkillInfoTable->FindRow<FSkillInfo>(FName(rowName), rowName);
			if (skillInfo == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::GetSkillInfoData %d is not found"), TargetSkillID);
				return FSkillInfo();
			}
			skillInfo->bIsValid = true;
			return SkillInfoCacheMap.Add(TargetSkillID, *skillInfo);
		}
		
	}
	return SkillInfoCacheMap[TargetSkillID];
}

TArray<int32> USkillManagerComponentBase::GetPlayerSkillIDList()
{
	if(!PlayerSkillIDList.IsEmpty()) return PlayerSkillIDList;
	InitSkillInfoCache();
	return PlayerSkillIDList;
}

float USkillManagerComponentBase::GetTotalCoolTime(int32 TargetSkillID)
{
	if (!CoolTimerHandleMap.Contains(TargetSkillID)) return 0.0001f;
	return GetWorld()->GetTimerManager().GetTimerRemaining(CoolTimerHandleMap[TargetSkillID])
			+ GetWorld()->GetTimerManager().GetTimerElapsed(CoolTimerHandleMap[TargetSkillID]);
}

float USkillManagerComponentBase::GetRemainingCoolTime(int32 TargetSkillID)
{
	if (!CoolTimerHandleMap.Contains(TargetSkillID)) return 0.0f;
	return GetWorld()->GetTimerManager().GetTimerRemaining(CoolTimerHandleMap[TargetSkillID]);
}

void USkillManagerComponentBase::GetCoolTimeVariables(int32 TargetSkillID, float& RemainingTime, float& TotalTime)
{
	RemainingTime = GetRemainingCoolTime(TargetSkillID);
	TotalTime = GetTotalCoolTime(TargetSkillID);
}


bool USkillManagerComponentBase::UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	return true;
}

void USkillManagerComponentBase::ClearAllSkill() 
{

}

void USkillManagerComponentBase::UpdateSkillValidity(int32 TargetSkillID, bool NewState)
{
	bool bIsInInventory = false;
	FSkillInfo skillInfo = GetSkillInfoData(TargetSkillID, bIsInInventory);
	if (!bIsInInventory || !OwnerCharacterRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::UpdateSkillValidity %d %d failed - Reason %d %d"), TargetSkillID, (int32)NewState, !bIsInInventory, !OwnerCharacterRef.IsValid());
		return;
	}
	SkillInventory[TargetSkillID].bIsValid = NewState;
}
