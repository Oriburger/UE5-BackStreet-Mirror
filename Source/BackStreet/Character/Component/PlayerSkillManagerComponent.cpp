// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSkillManagerComponent.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SkillSystem/SkillBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"	
#include "../../Character/Component/WeaponComponentBase.h"

UPlayerSkillManagerComponent::UPlayerSkillManagerComponent()
{
}

void UPlayerSkillManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerSkillManagerComponent::InitSkillManager()
{
	Super::InitSkillManager();
}

void UPlayerSkillManagerComponent::InitSkillMap()
{
	Super::InitSkillMap();
	if (!OwnerCharacterRef.IsValid()) return;

	SkillInventoryMap.Reset();
	EquipedSkillMap.Reset();

	for (ESkillType skillType : OwnerCharacterRef->WeaponComponent->GetWeaponStat().SkillTypeList)
	{
		SkillInventoryMap.Add(skillType, FSkillListContainer());
		EquipedSkillMap.Add(skillType, 0);
		ObtainableSkillMap.Add(skillType, FObtainableSkillListContainer());
	}
	UpdateObtainableSkillMap();
	return;
}

bool UPlayerSkillManagerComponent::TrySkill(int32 SkillID)
{
	return Super::TrySkill(SkillID);
}

bool UPlayerSkillManagerComponent::AddSkill(int32 SkillID)
{
	if (!Super::AddSkill(SkillID)) return false;
	FString rowName = FString::FromInt(SkillID);
	FSkillStatStruct* skillStat = SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
	checkf(skillStat != nullptr, TEXT("SkillStat is not valid"));

	ASkillBase* skillBase = Cast<ASkillBase>(GetWorld()->SpawnActor(skillStat->SkillBaseClassRef));
	if (!IsValid(skillBase)) return false;
	skillBase->InitSkill(*skillStat, this);

	if (skillBase->SkillStat.SkillWeaponStruct.SkillType == ESkillType::E_None) return false;
	ESkillType skillType = skillBase->SkillStat.SkillWeaponStruct.SkillType;

	//SkillInventoryMap에 추가
	if (!SkillInventoryMap.Contains(skillType)) false;
	FSkillListContainer* skillListContainer = SkillInventoryMap.Find(skillType);
	if (skillListContainer)
	{
		skillListContainer->SkillBaseList.AddUnique(skillBase);
	}

	//EquipedSkillMap에 장착되어 있는 스킬이 없다면 추가
	UE_LOG(LogTemp, Log, TEXT("skilltype : %d"), skillType);
	UE_LOG(LogTemp, Log, TEXT("skilltype : %d"), *EquipedSkillMap.Find(skillType));
	if (*EquipedSkillMap.Find(skillType) == 0)
	{
		EquipSkill(SkillID);
	}

	//KeepSkillList에 등록되어 있다면 삭제
	for (int32 keepSkill : KeepSkillList)
	{
		if (SkillID == keepSkill)
		{
			KeepSkillList.Remove(keepSkill);
			break;
		}
	}
	if (skillBase->SkillStat.SkillLevelStatStruct.bIsLevelValid)
	{
		UpgradeSkill(SkillID, 1);
	}
	UpdateObtainableSkillMap();
	return true;
}

bool UPlayerSkillManagerComponent::RemoveSkill(int32 SkillID)
{
	if (!IsSkillValid(SkillID)) return true;
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if (!IsValid(skillBase))
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::RemoveSkill(%d) : Skill Base Not Found"), SkillID);
		return false;
	}
	if (skillBase->SkillStat.SkillWeaponStruct.SkillType != ESkillType::E_None)
	{
		ESkillType skillType = skillBase->SkillStat.SkillWeaponStruct.SkillType;
		//EquipedSkillMap에 존재한다면 삭제
		if (EquipedSkillMap.Contains(skillType))
		{
			EquipedSkillMap.Add(skillType, 0);
		}
		//SkillInventoryMap에서 삭제
		if (SkillInventoryMap.Contains(skillType))
		{
			SkillInventoryMap.Find(skillType)->SkillBaseList.Remove(skillBase);
		}
	}
	OnSkillUpdated.Broadcast();
	UpdateObtainableSkillMap();
	return true;
}

bool UPlayerSkillManagerComponent::UpgradeSkill(int32 SkillID, uint8 NewLevel)
{
	return Super::UpgradeSkill(SkillID, NewLevel);
}

void UPlayerSkillManagerComponent::UpdateObtainableSkillMap()
{
	if (OwnerCharacterRef->WeaponComponent->WeaponID == 0) return;
	TArray<ESkillType> skillTypeList = OwnerCharacterRef->WeaponComponent->GetWeaponStat().SkillTypeList;

	static const FString ContextString(TEXT("GENERAL"));
	TArray<FSkillStatStruct*> allRows;
	SkillStatTable->GetAllRows(ContextString, allRows);
	TMap<ESkillType, FObtainableSkillListContainer> obtainableSkillMap;
	FObtainableSkillListContainer list1, list2, list3;
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
		TArray<int32> inventorySkillListByType;
		for (TWeakObjectPtr<ASkillBase> skillBase : SkillInventoryMap.Find(skillType)->SkillBaseList)
		{
			inventorySkillListByType.Add(skillBase->SkillStat.SkillID);
		}
		for (int32 skillID : inventorySkillListByType)
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

bool UPlayerSkillManagerComponent::EquipSkill(int32 NewSkillID)
{
	if (!IsSkillValid(NewSkillID)) return false;
	else
	{
		EquipedSkillMap.Add(GetSkillInfo(NewSkillID).SkillWeaponStruct.SkillType, NewSkillID);
		return true;
	}
}

void UPlayerSkillManagerComponent::ClearAllSkill()
{
	InitSkillMap();
}

bool UPlayerSkillManagerComponent::IsSkillValid(int32 SkillID)
{
	return Super::IsSkillValid(SkillID);
}

ASkillBase* UPlayerSkillManagerComponent::GetOwnSkillBase(int32 SkillID)
{
	if (!SkillInventoryMap.Contains(GetSkillTypeInfo(SkillID))) return nullptr;
	for (TWeakObjectPtr<ASkillBase> skillBase : SkillInventoryMap.Find(GetSkillTypeInfo(SkillID))->SkillBaseList)
	{
		if (skillBase->SkillStat.SkillID == SkillID)
		{
			return skillBase.Get();
		}
	}
	return nullptr;
}

TArray<uint8> UPlayerSkillManagerComponent::GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillStat.SkillLevelStatStruct.LevelInfo[NewSkillLevel].RequiredMaterial;
}
