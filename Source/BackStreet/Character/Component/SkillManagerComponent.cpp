// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerComponent.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SkillSystem/SkillBase.h"

// Sets default values for this component's properties
USkillManagerComponent::USkillManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void USkillManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	InitSkillManager();
}

void USkillManagerComponent::InitSkillManager()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	SkillStatTable = GameModeRef.Get()->SkillStatTable;
	checkf(IsValid(SkillStatTable), TEXT("Failed to get SkillStatDataTable"));
}

bool USkillManagerComponent::TrySkill(int32 SkillID)
{
	ASkillBase* skillBase = GetSkillBase(SkillID);
	if(skillBase == nullptr) return false;
	if (!skillBase->SkillState.bIsBlocked) return false;
	if (skillBase->SkillStat.SkillLevelStatStruct.bIsLevelValid)
	{
		if(skillBase->SkillState.SkillLevelStateStruct.SkillLevel == 0) return false;
	}
	OwnerCharacterRef->SetActionState(ECharacterActionType::E_Skill);
	skillBase->ActivateSkill();

	return true;
}

bool USkillManagerComponent::AddSkill(int32 SkillID)
{
	if (SkillMap.Contains(SkillID)) return false;
	FString rowName = FString::FromInt(SkillID);
	FSkillStatStruct skillStat = *SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
	TWeakObjectPtr<ASkillBase> skillBase = Cast<ASkillBase>(GetWorld()->SpawnActor(skillStat.SkillBaseClassRef));
	SkillMap.Add(SkillID, skillBase);
	if (skillBase->SkillStat.SkillWeaponStruct.SkillType != ESkillType::E_None)
	{
		ESkillType skillType = skillBase->SkillStat.SkillWeaponStruct.SkillType;
		if (SkillInventoryMap.Contains(skillType))
		{
			SkillInventoryMap.Find(skillType)->SkillIDList.AddUnique(SkillID);
		}
		else
		{
			FSkillInventoryContainer inventory;
			inventory.SkillIDList.AddUnique(SkillID);
			SkillInventoryMap.Add(skillType, FSkillInventoryContainer{ inventory });
		}
	}
	skillBase->InitSkill(skillStat, this);
	return true;
}

bool USkillManagerComponent::RemoveSkill(int32 SkillID)
{
	if (!SkillMap.Contains(SkillID)) return true;
	ASkillBase* skillBase = SkillMap.Find(SkillID)->Get();
	if (skillBase->SkillStat.SkillWeaponStruct.SkillType != ESkillType::E_None)
	{
		ESkillType skillType = skillBase->SkillStat.SkillWeaponStruct.SkillType;
		if (SkillInventoryMap.Contains(skillType))
		{
			SkillInventoryMap.Find(skillType)->SkillIDList.Remove(SkillID);
		}
	}
	SkillMap.Remove(SkillID);
	return true;
}

TArray<int32> USkillManagerComponent::GetOwnSkillID()
{
	if(SkillMap.IsEmpty()) return TArray<int32>();
	TArray<int32> keys;
	SkillMap.GetKeys(keys);
	return keys;
}

ASkillBase* USkillManagerComponent::GetSkillBase(int32 SkillID)
{
	if (!SkillMap.Contains(SkillID)) return nullptr;
	return SkillMap.Find(SkillID)->Get();
}

bool USkillManagerComponent::IsSkillValid(int32 SkillID)
{
	if (!SkillMap.Contains(SkillID)) return false;
	return true;
}

FSkillStatStruct USkillManagerComponent::GetSkillStat(int32 SkillID)
{
	ASkillBase* skillBase = GetSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillStat;
}

FSkillStateStruct USkillManagerComponent::GetSkillState(int32 SkillID)
{
	ASkillBase* skillBase = GetSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillState;
}

TArray<uint8> USkillManagerComponent::GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel)
{
	ASkillBase* skillBase = GetSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillStat.SkillLevelStatStruct.RequiredMaterialsByLevel[NewSkillLevel].RequiredMaterial;
}

TArray<FSkillInventoryContainer> USkillManagerComponent::GetObtainableSkillList(int32 WeaponID)
{

	return TArray<FSkillInventoryContainer>();
}




