// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerComponent.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SkillSystem/SkillBase.h"
#include "../../Character/Component/WeaponComponentBase.h"

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
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
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
			FSkillListContainer inventory;
			inventory.SkillIDList.AddUnique(SkillID);
			SkillInventoryMap.Add(skillType, FSkillListContainer{ inventory });
		}
	}
	skillBase->InitSkill(skillStat, this);
	UpdateObtainableSkillMap();
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
	UpdateObtainableSkillMap();
	return true;
}

void USkillManagerComponent::UpdateObtainableSkillMap()
{
	if (OwnerCharacterRef->WeaponComponent->WeaponID == 0) return;
	TArray<ESkillType> skillTypeList = OwnerCharacterRef->WeaponComponent->GetWeaponStat().SkillTypeList;

	static const FString ContextString(TEXT("GENERAL"));
	TArray<FSkillStatStruct*> allRows;
	SkillStatTable->GetAllRows(ContextString, allRows);
	TMap<ESkillType, FSkillListContainer> obtainableSkillMap;
	FSkillListContainer list1, list2, list3;
	obtainableSkillMap.Add(skillTypeList[0], list1);
	obtainableSkillMap.Add(skillTypeList[1], list2);
	obtainableSkillMap.Add(skillTypeList[2], list3);

	//SkillStatTable에서 전체 스킬의 타입을 검사하여 무기에 사용되는 스킬 타입과 비교 후 저장
	for (FSkillStatStruct* row : allRows)
	{
		for (ESkillType skillType : skillTypeList)
		{
			if (row->SkillWeaponStruct.SkillType == skillType)
			{
				obtainableSkillMap[skillType].SkillIDList.Add(row->SkillID);
			}
		}
	}

	//현재 지닌 스킬을 배열에서 제거
	for (ESkillType skillType : skillTypeList)
	{
		if (!SkillInventoryMap.Contains(skillType)) continue;
		TArray<int32> inventorySkillList = SkillInventoryMap.Find(skillType)->SkillIDList;
		for (int32 skillID : inventorySkillList)
		{
			if (obtainableSkillMap[skillType].SkillIDList.Contains(skillID))
			{
				obtainableSkillMap[skillType].SkillIDList.Remove(skillID);
			}
		}
	}

	//스킬제작UI에서 제시되었던 스킬 제외
	for (int32 skillID : DisplayedSkillList)
	{
		ESkillType displayedSkillType = GetSkillInfo(skillID).SkillWeaponStruct.SkillType;
		for (ESkillType skillType : skillTypeList)
		{
			if (skillType == displayedSkillType)
			{
				if (obtainableSkillMap[skillType].SkillIDList.Contains(skillID))
				{
					obtainableSkillMap[skillType].SkillIDList.Remove(skillID);
				}
			}
		}
	}

	ObtainableSkillMap = obtainableSkillMap;
	return;
}

FSkillStatStruct USkillManagerComponent::GetSkillInfo(int32 SkillID)
{
	FString rowName = FString::FromInt(SkillID);
	return *SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
}

ASkillBase* USkillManagerComponent::GetOwnSkillBase(int32 SkillID)
{
	if (!SkillMap.Contains(SkillID)) return nullptr;
	return SkillMap.Find(SkillID)->Get();
}

bool USkillManagerComponent::IsSkillValid(int32 SkillID)
{
	if (!SkillMap.Contains(SkillID)) return false;
	return true;
}

FSkillStatStruct USkillManagerComponent::GetOwnSkillStat(int32 SkillID)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillStat;
}

FSkillStateStruct USkillManagerComponent::GetOwnSkillState(int32 SkillID)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillState;
}

TArray<uint8> USkillManagerComponent::GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillStat.SkillLevelStatStruct.RequiredMaterialsByLevel[NewSkillLevel].RequiredMaterial;
}