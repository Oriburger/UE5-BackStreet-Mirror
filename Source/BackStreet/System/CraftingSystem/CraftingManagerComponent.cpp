// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerComponent.h"
#include "CraftBoxBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/SkillManagerComponent.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"

UCraftingManagerComponent::UCraftingManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UCraftingManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	InitCraftingManager();
	
}

void UCraftingManagerComponent::InitCraftingManager()
{
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	OwnerActorRef = Cast<ACraftBoxBase>(GetOwner());
	MainCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	SkillManagerRef = MainCharacterRef->SkillManagerComponent;
	bIsSkillCreated = false;
}

bool UCraftingManagerComponent::IsMatEnough()
{
	TArray<uint8> currMatList = MainCharacterRef->ItemInventory->GetCraftingItemAmount();
	for (uint8 requiredMatIdx = 0 ; requiredMatIdx < RequiredMatList.Num(); requiredMatIdx++)
	{
		//������� ��� �� ���� ���� Ȯ��
		if (KeepMatIdx == requiredMatIdx && 
			currMatList[4] < RequiredMatList[requiredMatIdx]) return false;
		//�Ϲ���� ��� �� ���� ���� Ȯ��
		if(currMatList[requiredMatIdx] < RequiredMatList[requiredMatIdx]) return false;
	}
	return true;
}

bool UCraftingManagerComponent::ConsumeMat()
{
	for (uint8 idx = 0; idx < MAX_CRAFTING_ITEM_IDX; idx++)
	{
		//������� ��� ��
		if (KeepMatIdx == idx)
		{
			MainCharacterRef->ItemInventory->RemoveItem(4, RequiredMatList[idx]);
		}
		//�⺻��� ��� ��
		else
		{
			MainCharacterRef->ItemInventory->RemoveItem(idx + 1, RequiredMatList[idx]);
		}
	}
	return false;
}

bool UCraftingManagerComponent::AddSkill(int32 NewSkillID)
{	
	//��� ������� Ȯ��
	UpdateRequiredMatForSU(NewSkillID, 1);
	if (!IsMatEnough())return false;
	
	//��ų �߰�
	bool bIsAddSkillSucceed = SkillManagerRef->AddSkill(NewSkillID);
	if (!bIsAddSkillSucceed) return false;
	else
	{
		//��� �Ҹ�
		ConsumeMat();
		//��ų ����UI���� ���� �Ǿ��� ��ų ����Ʈ�� ���
		TArray<int32> displayedSkillIDList;
		DisplayingSkillMap.GenerateValueArray(displayedSkillIDList);
		for (int32 skillID : displayedSkillIDList)
		{
			SkillManagerRef->DisplayedSkillList.Add(skillID);
		}

		//��ų �ߺ� �߰� ����
		bIsSkillCreated = true;
		return true;
	}
}

void UCraftingManagerComponent::SetDisplayingSkillList()
{
	DisplayingSkillMap.Reset();
	TArray<ESkillType> skillTypeList = MainCharacterRef->WeaponComponent->WeaponStat.SkillTypeList;
	//DisplayingSkillMap �ʱ�ȭ
	for (ESkillType skillType : skillTypeList)
	{
		DisplayingSkillMap.Add(skillType,0);
	}

	//�� �Ǿ��ִ� ��ų Ÿ���� �ִ��� Ȯ��
	TArray<int32> keepSkillList = SkillManagerRef->KeepSkillList;
	for (int32 keepSkill : keepSkillList)
	{
		for (ESkillType skillType : skillTypeList)
		{
			if (SkillManagerRef->GetSkillInfo(keepSkill).SkillWeaponStruct.SkillType == skillType)
			{
				DisplayingSkillMap.Add(skillType, keepSkill);
			}
		}
	}

	//�� �ȵǾ� �ִ� ��ų Ÿ���� �������� ����ġ ����Ͽ� ����
	for (ESkillType skillType : skillTypeList)
	{
		if(DisplayingSkillMap.Find(skillType)!=0) continue;
		else 
		{
			TMap<ESkillType, FObtainableSkillListContainer>obtainableSkillMap = SkillManagerRef->GetObtainableSkillMap();
			checkf(obtainableSkillMap.Contains(skillType), TEXT("ObtainableSkillMap is something wrong"));
			//����ġ ���� ����
			int32 totalWeight = 0;
			for (int32 skillID : obtainableSkillMap[skillType].SkillIDList)
			{
				totalWeight += SkillManagerRef->GetSkillInfo(skillID).SkillWeaponStruct.RandomWeight;
			}
			int32 pivot = FMath::RandRange(0, totalWeight);
			int32 tempWeight = 0;
			for (int32 skillID : obtainableSkillMap[skillType].SkillIDList)
			{
				tempWeight += SkillManagerRef->GetSkillInfo(skillID).SkillWeaponStruct.RandomWeight;
				if (tempWeight >= pivot)
				{
					DisplayingSkillMap.Add(skillType, skillID);
					break;
				}
			}
		}
	}
}

void UCraftingManagerComponent::KeepSkill(int32 SkillID)
{
	SkillManagerRef->KeepSkillList.Add(SkillID);
}

void UCraftingManagerComponent::UnkeepSkill(int32 SkillID)
{
	SkillManagerRef->KeepSkillList.Remove(SkillID);
}

bool UCraftingManagerComponent::UpgradeSkill(int32 SkillID, uint8 NewLevel)
{
	//��� ������� Ȯ��
	UpdateRequiredMatForSU(SkillID, NewLevel);
	if(!IsMatEnough())return false;

	//��ų �߰�
	bool bIsAddSkillSucceed = SkillManagerRef->UpgradeSkill(SkillID, NewLevel);
	if (!bIsAddSkillSucceed) return false;
	else
	{
		//��� �Ҹ�
		ConsumeMat();
		return true;
	}
}

TArray<uint8> UCraftingManagerComponent::UpdateRequiredMatForSU(int32 SkillID, uint8 NewLevel)
{
	RequiredMatList.Empty();
	uint8 currSkillLevel = SkillManagerRef->GetOwnSkillState(SkillID).SkillLevelStateStruct.SkillLevel;
	
	for (uint8 idx = 0; idx < MAX_CRAFTING_ITEM_IDX; idx++)
	{
		uint8 totalAmt = 0;
		for (uint8 currLevel = currSkillLevel + 1; currLevel <= NewLevel; currLevel++)
		{
			totalAmt += SkillManagerRef->GetSkillInfo(SkillID).SkillLevelStatStruct.RequiredMaterialsByLevel[currLevel].RequiredMaterial[idx];
		}
		RequiredMatList.Add(totalAmt);
	}
	return RequiredMatList;
}

bool UCraftingManagerComponent::UpgradeWeapon(TArray<uint8> NewLevelList)
{
	//��� ������� Ȯ��
	UpdateRequiredMatForWU(NewLevelList);
	if (!IsMatEnough())return false;

	//��� �Ҹ�
	ConsumeMat();

	return MainCharacterRef->WeaponComponent->UpgradeStat(NewLevelList);
}

TArray<uint8> UCraftingManagerComponent::UpdateRequiredMatForWU(TArray<uint8> NewLevelList)
{
	RequiredMatList.Empty();
	UWeaponComponentBase* weaponRef = MainCharacterRef->WeaponComponent;
	TArray<uint8> currLevelList = weaponRef->WeaponState.UpgradedStatList;

	for (uint8 idx = 0; idx < MAX_WEAPON_UPGRADABLE_STAT_IDX; idx++)
	{
		for (uint8 idx2 = 0; idx2 < MAX_CRAFTING_ITEM_IDX; idx2++)
		{
			for (uint8 currLevel = currLevelList[idx] + 1; currLevel <= NewLevelList[idx2]; currLevel++)
			{
				RequiredMatList[idx2] += weaponRef->WeaponStat.UpgradableStatInfoList[idx].RequiredMaterialByLevel[NewLevelList[idx2]].RequiredMaterial[idx2];
			}
		}
	}
	return RequiredMatList;
}