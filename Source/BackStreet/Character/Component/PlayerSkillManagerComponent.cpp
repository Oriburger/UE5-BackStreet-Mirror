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
	if (!OwnerCharacterRef.IsValid()) return;
	if (OwnerCharacterRef->WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Melee) return;
	Super::InitSkillMap();

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

	//SkillInventoryMap�� �߰�
	if (!SkillInventoryMap.Contains(skillType)) false;
	FSkillListContainer* skillListContainer = SkillInventoryMap.Find(skillType);
	if (skillListContainer)
	{
		skillListContainer->SkillBaseList.AddUnique(skillBase);
	}

	//EquipedSkillMap�� �����Ǿ� �ִ� ��ų�� ���ٸ� �߰�
	UE_LOG(LogTemp, Log, TEXT("skilltype : %d"), skillType);
	UE_LOG(LogTemp, Log, TEXT("skilltype : %d"), *EquipedSkillMap.Find(skillType));
	if (*EquipedSkillMap.Find(skillType) == 0)
	{
		EquipSkill(SkillID);
	}

	//KeepSkillList�� ��ϵǾ� �ִٸ� ����
	for (int32 keepSkill : KeepSkillList)
	{
		if (SkillID == keepSkill)
		{
			KeepSkillList.Remove(keepSkill);
			break;
		}
	}
	if (skillBase->SkillStat.bIsLevelValid)
	{
		UpgradeSkill(SkillID, ESkillUpgradeType::E_CoolTime, 1);
	}
	UpdateObtainableSkillMap();
	OnSkillUpdated.Broadcast();
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
		//EquipedSkillMap�� �����Ѵٸ� ����
		if (EquipedSkillMap.Contains(skillType))
		{
			EquipedSkillMap.Add(skillType, 0);
		}
		//SkillInventoryMap���� ����
		if (SkillInventoryMap.Contains(skillType))
		{
			SkillInventoryMap.Find(skillType)->SkillBaseList.Remove(skillBase);
		}
	}
	UpdateObtainableSkillMap();
	OnSkillUpdated.Broadcast();
	return true;
}

bool UPlayerSkillManagerComponent::UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	bool bIsUpgraded = Super::UpgradeSkill(SkillID, UpgradeTarget, NewLevel);
	OnSkillUpdated.Broadcast();
	return bIsUpgraded;
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

	//SkillStatTable���� ��ü ��ų�� Ÿ���� �˻��Ͽ� ���⿡ ���Ǵ� ��ų Ÿ�԰� �� �� ����
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

	//���� ���� ��ų�� �迭���� ����
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

	//��ų����UI���� ���õǾ��� ��ų ����
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
	OnSkillUpdated.Broadcast();
	return;
}

bool UPlayerSkillManagerComponent::EquipSkill(int32 NewSkillID)
{
	if (!IsSkillValid(NewSkillID)) return false;
	else
	{
		EquipedSkillMap.Add(GetSkillInfo(NewSkillID).SkillWeaponStruct.SkillType, NewSkillID);
		OnSkillUpdated.Broadcast();
		return true;
	}
}

void UPlayerSkillManagerComponent::ClearAllSkill()
{
	Super::ClearAllSkill();
	InitSkillMap();
	OnSkillUpdated.Broadcast();
}

bool UPlayerSkillManagerComponent::IsSkillValid(int32 SkillID)
{
	return Super::IsSkillValid(SkillID);
}

bool UPlayerSkillManagerComponent::GetIsSkillUpgradable(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	return Super::GetIsSkillUpgradable(SkillID, UpgradeTarget, NewLevel);
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
	FSkillStatStruct skillInfo = GetSkillInfo(SkillID);
	checkf(skillInfo.SkillID != 0, TEXT("Failed Find Skill"));
	return {};//skillInfo.LevelInfo[NewSkillLevel].RequiredMaterial;
}

TMap<int32, int32> UPlayerSkillManagerComponent::GetRequiredMaterialInfo(int32 SkillID, ESkillUpgradeType UpgradeType, uint8 Level)
{
	FSkillStatStruct skillInfo = GetSkillInfo(SkillID);
	checkf(skillInfo.SkillID != 0, TEXT("Failed Find Skill"));
	if(!skillInfo.LevelInfo.Contains(UpgradeType)) return TMap<int32, int32>();
	else if(!skillInfo.LevelInfo[UpgradeType].SkillLevelInfoList.IsValidIndex(Level)) return TMap<int32, int32>();
	return skillInfo.LevelInfo[UpgradeType].SkillLevelInfoList[Level].RequiredMaterialMap;
}
