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

void USkillManagerComponentBase::InitSkillManager(FSkillManagerInfoStruct SkillManagerInfoOverride)
{
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	OwnerCharacterRef->WeaponComponent->OnMainWeaponUpdated.AddDynamic(this, &USkillManagerComponentBase::ClearAllSkill);

	if (SkillManagerInfoOverride.SkillInfoCacheMap.Num() > 0)
	{
		SkillManagerInfo = SkillManagerInfoOverride;

		//ID 2 's cooltime check
		UE_LOG(LogTemp, Warning, TEXT("USkillManagerComponentBase::InitSkillManager , Skill ID 2's remained cooltime : %.2lf"),
			SkillManagerInfo.CoolTimerHandleMap.Contains(2) ? GetWorld()->GetTimerManager().GetTimerRemaining(SkillManagerInfo.CoolTimerHandleMap[2]) : -1.0f);

		//Restore CoolTime Timer
		RestoreCoolTimeTimer();
	}
	else
	{
		//Init Skill Map
		InitSkillInfoCache();
	}
	
}


// Called when the game starts
void USkillManagerComponentBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitSkillManager();
}

void USkillManagerComponentBase::InitSkillInfoCache()
{
	if (!SkillManagerInfo.SkillInfoCacheMap.IsEmpty()) return;
	if (SkillInfoTable != nullptr)
	{
		TArray<FName> rowNameList = SkillInfoTable->GetRowNames();
		for (auto& row : rowNameList)
		{
			FString contextString = "";
			FSkillInfo* skillInfo = SkillInfoTable->FindRow<FSkillInfo>(row, contextString);
			if (skillInfo == nullptr) continue;
			SkillManagerInfo.SkillInfoCacheMap.Add(skillInfo->SkillID, *skillInfo);

			if (skillInfo->bIsPlayerSkill)
			{
				SkillManagerInfo.PlayerSkillIDList.Add(skillInfo->SkillID);
			}
		}
	}
}

void USkillManagerComponentBase::RestoreCoolTimeTimer()
{
	if (SkillManagerInfo.SkillInfoCacheMap.IsEmpty() || SkillManagerInfo.CoolTimerHandleMap.IsEmpty())
	{
		UE_LOG(LogSaveSystem, Error, TEXT("USkillManagerComponentBase::RestoreCoolTimeTimer - SkillInfoCacheMap or CoolTimerHandleMap is empty"));
		return;
	}

	for (auto& valueInfo : SkillManagerInfo.CoolTimeValueMap)
	{
		const int32 skillID = valueInfo.Key;
		const float remainedCoolTime = GetRemainingCoolTime(valueInfo.Value);

		if (!SkillManagerInfo.CoolTimerHandleMap.Contains(skillID))
		{
			UE_LOG(LogSaveSystem, Error, TEXT("USkillManagerComponentBase::RestoreCoolTimeTimer - CoolTimerHandleMap does not contain skillID %d"), skillID);
			continue;
		}
		
		FTimerDelegate timerDelegate;
		timerDelegate.BindUFunction(this, FName("UpdateSkillValidity"), skillID, true);

		SkillManagerInfo.CoolTimerHandleMap[skillID].Invalidate();
		GetWorld()->GetTimerManager().SetTimer(SkillManagerInfo.CoolTimerHandleMap[skillID], timerDelegate, remainedCoolTime, false);
		UE_LOG(LogTemp, Log, TEXT("USkillManagerComponentBase::RestoreCoolTimeTimer - SkillID %d, Remained CoolTime : %.2f"), skillID, remainedCoolTime);
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
	SkillManagerInfo.SkillInventory.Add(NewSkillID, skillInfo);

	if (skillInfo.CoolTimeValue > 0.0f && !SkillManagerInfo.CoolTimerHandleMap.Contains(NewSkillID))
	{
		SkillManagerInfo.CoolTimerHandleMap.Add(NewSkillID, FTimerHandle());
	}

	TArray<FSkillInfo> skillInfoList;
	SkillManagerInfo.SkillInventory.GenerateValueArray(skillInfoList);
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
	SkillManagerInfo.SkillInventory.Remove(TargetSkillID);
	if (skillInfo.CoolTimeValue > 0.0f && SkillManagerInfo.CoolTimerHandleMap.Contains(TargetSkillID))
	{
		SkillManagerInfo.CoolTimerHandleMap[TargetSkillID].Invalidate();
		GetWorld()->GetTimerManager().ClearTimer(SkillManagerInfo.CoolTimerHandleMap[TargetSkillID]);
		SkillManagerInfo.CoolTimerHandleMap.Remove(TargetSkillID);
	}
	TArray<FSkillInfo> skillInfoList;
	SkillManagerInfo.SkillInventory.GenerateValueArray(skillInfoList);
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
	
	if (SkillManagerInfo.PrevSkillInfo.PrevActionRef.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("USkilllManagercomponentBase::TryActivateSkill prevSkill is not Destroyed - Reason %d"), !SkillManagerInfo.PrevSkillInfo.PrevActionRef.IsValid());
		SkillManagerInfo.PrevSkillInfo.PrevActionRef.Get()->Destroy();
	}

	if (!skillInfo.bContainPrevAction)
	{
		OwnerCharacterRef.Get()->PlayAnimMontage(skillInfo.SkillAnimation);
		SkillManagerInfo.PrevSkillInfo = skillInfo;
	}
	else if (skillInfo.PrevActionClass != nullptr)
	{
		ASkillBase* prevAction = Cast<ASkillBase>(GetWorld()->SpawnActor(skillInfo.PrevActionClass));
		prevAction->InitSkill(FSkillStatStruct(), this, skillInfo); //제거 필요
		prevAction->ActivateSkill();
		SkillManagerInfo.PrevSkillInfo = skillInfo;
	}
	else return false;

	OnSkillActivated.Broadcast(skillInfo);

	if (skillInfo.CoolTimeValue > 0.0f && SkillManagerInfo.CoolTimerHandleMap.Contains(TargetSkillID))
	{
		FTimerDelegate timerDelegate;
		timerDelegate.BindUFunction(this, FName("UpdateSkillValidity"), TargetSkillID, true);
		UpdateSkillValidity(TargetSkillID, false);
		const float coolTimeAbilityValue = OwnerCharacterRef.Get()->GetStatTotalValue(ECharacterStatType::E_SkillCoolTime) - 1.0f;
		const float totalCoolTimeValue = FMath::Clamp(skillInfo.CoolTimeValue - skillInfo.CoolTimeValue * coolTimeAbilityValue, 0.1f, 60.0f);
		GetWorld()->GetTimerManager().SetTimer(SkillManagerInfo.CoolTimerHandleMap[TargetSkillID], timerDelegate, totalCoolTimeValue, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USkillManagerComponentBase::TryActivateSkill remain cooltime : %.2lf, contains : %d"),
			GetRemainingCoolTime(TargetSkillID), SkillManagerInfo.CoolTimerHandleMap.Contains(TargetSkillID));
	}

	return true;
}

void USkillManagerComponentBase::DeactivateCurrentSkill()
{
	if (SkillManagerInfo.PrevSkillInfo.PrevActionRef.IsValid())
	{
		ASkillBase* prevSkillBase = Cast<ASkillBase>(SkillManagerInfo.PrevSkillInfo.PrevActionRef);
		prevSkillBase->DeactivateSkill();
		prevSkillBase->Destroy();
	}
	SkillManagerInfo.PrevSkillInfo.IsValid();
	OnSkillDeactivated.Broadcast(SkillManagerInfo.PrevSkillInfo);

	SkillManagerInfo.PrevSkillInfo = FSkillInfo();
}

TArray<int32> USkillManagerComponentBase::GetCraftableIDList()
{
	TArray<int32> resultList;

	for (int32& id : SkillManagerInfo.PlayerSkillIDList)
	{
		if (!SkillManagerInfo.SkillInventory.Contains(id))
		{
			resultList.Add(id);
		}
	}
	return resultList;
}

FSkillInfo USkillManagerComponentBase::GetSkillInfoData(int32 TargetSkillID, bool& bIsInInventory)
{
	if (SkillManagerInfo.SkillInventory.Contains(TargetSkillID))
	{
		bIsInInventory = true;
		return SkillManagerInfo.SkillInventory[TargetSkillID];
	}

	//Enemy doesn't use the inventory data. 
	bIsInInventory = OwnerCharacterRef.IsValid() ? OwnerCharacterRef.Get()->ActorHasTag("Enemy") : false;
	FString rowName = FString::FromInt(TargetSkillID);
	if (!SkillManagerInfo.SkillInfoCache.Contains(TargetSkillID))
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
			return SkillManagerInfo.SkillInfoCacheMap.Add(TargetSkillID, *skillInfo);
		}
		
	}
	return SkillManagerInfo.SkillInfoCacheMap[TargetSkillID];
}

TArray<int32> USkillManagerComponentBase::GetPlayerSkillIDList()
{
	if(!SkillManagerInfo.PlayerSkillIDList.IsEmpty()) return SkillManagerInfo.PlayerSkillIDList;
	InitSkillInfoCache();
	return SkillManagerInfo.PlayerSkillIDList;
}

float USkillManagerComponentBase::GetTotalCoolTime(int32 TargetSkillID)
{
	if (!SkillManagerInfo.CoolTimerHandleMap.Contains(TargetSkillID)) return 0.0001f;
	return GetWorld()->GetTimerManager().GetTimerRemaining(SkillManagerInfo.CoolTimerHandleMap[TargetSkillID])
			+ GetWorld()->GetTimerManager().GetTimerElapsed(SkillManagerInfo.CoolTimerHandleMap[TargetSkillID]);
}

float USkillManagerComponentBase::GetRemainingCoolTime(int32 TargetSkillID)
{
	if (!SkillManagerInfo.CoolTimerHandleMap.Contains(TargetSkillID)) return 0.0f;
	return GetWorld()->GetTimerManager().GetTimerRemaining(SkillManagerInfo.CoolTimerHandleMap[TargetSkillID]);
}

void USkillManagerComponentBase::GetCoolTimeVariables(int32 TargetSkillID, float& RemainingTime, float& TotalTime)
{
	RemainingTime = GetRemainingCoolTime(TargetSkillID);
	TotalTime = GetTotalCoolTime(TargetSkillID);
}

FSkillManagerInfoStruct USkillManagerComponentBase::GetSkillManagerInfo()
{
	//capture cool time value for each skill
	UE_LOG(LogSaveSystem, Log, TEXT("USkillManagerComponentBase::GetSkillManagerInfo - CoolTimerHandleMap Count : %d"), SkillManagerInfo.CoolTimerHandleMap.Num());
	for (auto& timerRef : SkillManagerInfo.CoolTimerHandleMap)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(timerRef.Value))
		{
			SkillManagerInfo.CoolTimeValueMap.Add(timerRef.Key, GetWorld()->GetTimerManager().GetTimerRemaining(timerRef.Value));
			UE_LOG(LogSaveSystem, Log, TEXT("USkillManagerComponentBase::GetSkillManagerInfo - SkillID : %d, CoolTimeValue : %.2f"), timerRef.Key, SkillManagerInfo.CoolTimeValueMap[timerRef.Key]);
		}
	}

	if (SkillManagerInfo.SkillInfoCacheMap.Num() > 0)
	{
		return SkillManagerInfo;
	}
	return FSkillManagerInfoStruct();
}


bool USkillManagerComponentBase::UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	return true;
}

void USkillManagerComponentBase::ClearAllSkill() 
{

}

void USkillManagerComponentBase::StopSkillAnimMontage()
{	
	UAnimInstance* animInstance = OwnerCharacterRef->GetMesh()->GetAnimInstance();
	if (IsValid(animInstance) && animInstance->IsAnyMontagePlaying()) 
	{
		animInstance->Montage_Stop(0.0f);
		DeactivateCurrentSkill();
	}
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
	SkillManagerInfo.SkillInventory[TargetSkillID].bIsValid = NewState;
}
