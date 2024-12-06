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
	return true;
}

void USkillManagerComponentBase::DeactivateCurrentSkill()
{
	ASkillBase* prevSkillBase;
	if (PrevSkillInfo.PrevActionRef.IsValid())
	{
		prevSkillBase = Cast<ASkillBase>(PrevSkillInfo.PrevActionRef);
		prevSkillBase->DeactivateSkill();
		prevSkillBase->Destroy();
	}
	PrevSkillInfo.IsValid();
	OnSkillDeactivated.Broadcast(PrevSkillInfo);

	PrevSkillInfo = FSkillInfo();
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
			return SkillInfoCacheMap.Add(TargetSkillID, *skillInfo);
		}
		
	}
	return SkillInfoCacheMap[TargetSkillID];
}


bool USkillManagerComponentBase::UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	return true;
}

void USkillManagerComponentBase::ClearAllSkill() 
{

}
