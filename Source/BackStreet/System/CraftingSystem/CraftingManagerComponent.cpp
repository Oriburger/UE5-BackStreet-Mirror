// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerComponent.h"
#include "CraftBoxBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/SkillManagerComponentBase.h"
#include "../../Character/Component/PlayerSkillManagerComponent.h"
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
	OwnerActorRef = GetOwner();
	OwnerActorRef->Tags.Add("CraftingBox");
	MainCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	SkillManagerRef = MainCharacterRef.IsValid() ? MainCharacterRef->SkillManagerComponent : nullptr;
	bIsSkillCreated = false;
	bIsDisplayingSkillListSet = false;
}

bool UCraftingManagerComponent::GetIsItemEnough()
{
	if(MainCharacterRef->GetCharacterStat().bInfiniteSkillMaterial) return true;
	TMap<ECraftingItemType, uint8> currItemMap = MainCharacterRef->ItemInventory->GetCraftingItemAmount();
	
	TArray<int32> itemIdList; RequiredItemInfo.GenerateKeyArray(itemIdList);
	for (int32 requiredItemId : itemIdList)
	{
		//������� ��� �� ���� ���� Ȯ��
		if (static_cast<uint8>(KeptItem) == requiredItemId + 1 &&
			currItemMap[ECraftingItemType::E_Wrench] >= RequiredItemInfo[requiredItemId]);
		//�Ϲ���� ��� �� ���� ���� Ȯ��
		else
		{
			if (currItemMap[(ECraftingItemType)requiredItemId] < RequiredItemInfo[requiredItemId])
				return false;
		}
	}
	return true;
}

bool UCraftingManagerComponent::GetIsItemEnoughForCraft(int32 SkillID)
{
	if (MainCharacterRef->GetCharacterStat().bInfiniteSkillMaterial) return true;
	TMap<int32, int32> craftMaterialInfo = GetSkillCraftMaterialInfo(SkillID);
	TMap<ECraftingItemType, uint8> currItemMap = MainCharacterRef->ItemInventory->GetCraftingItemAmount();
	TArray<int32> idList; craftMaterialInfo.GenerateKeyArray(idList);

	for (int32& id : idList) 
	{
		if (!currItemMap.Contains((ECraftingItemType)id)) return false;
		const int32 requiredCount = craftMaterialInfo[id];
		if (requiredCount > currItemMap[(ECraftingItemType)id]) return false;
	}
	return true;
}

bool UCraftingManagerComponent::ConsumeItem()
{
	if (MainCharacterRef->GetCharacterStat().bInfiniteSkillMaterial) return true;

	TArray<int32> itemIdList; RequiredItemInfo.GenerateKeyArray(itemIdList);
	for (int32 requiredItemId : itemIdList)
	{
		//������� ��� ��
	//	if (static_cast<uint8>(KeptItem) == requiredItemId + 1)
	//	{
	//		MainCharacterRef->ItemInventory->RemoveItem(4, RequiredItemInfo[requiredItemId]);
	//	}
		//�⺻��� ��� ��
	//	else
		{
			MainCharacterRef->ItemInventory->RemoveItem(requiredItemId, RequiredItemInfo[requiredItemId]);
		}
	}
	return true;
}

bool UCraftingManagerComponent::GetIsSkillKeepingAvailable()
{
	if (!SkillManagerRef.IsValid()) return false;
	if(SkillManagerRef->KeepSkillList.Num()>MainCharacterRef->GetCharacterStat().MaxKeepingSkillCount	) return false;
	return true;
}

void UCraftingManagerComponent::KeepItem(EKeepMat NewKeptItem)
{
	KeptItem = NewKeptItem;
}

void UCraftingManagerComponent::UnKeepItem()
{
	KeptItem = EKeepMat::E_None;
}

bool UCraftingManagerComponent::AddSkill(int32 NewSkillID)
{	
	if (!SkillManagerRef.IsValid()) return false;

	UpdateSkillCraftRequiredItemList(NewSkillID);
	if (!GetIsItemEnough())return false;
	
	//��ų �߰�
	bool bIsAddSkillSucceed = SkillManagerRef->AddSkill(NewSkillID);
	if (!bIsAddSkillSucceed) return false;
	else
	{
		//��� �Ҹ�
		ConsumeItem();
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
	if (!SkillManagerRef.IsValid()) return;
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
		if(*DisplayingSkillMap.Find(skillType)!=0) continue;
		else 
		{
			TMap<ESkillType, FObtainableSkillListContainer> obtainableSkillMap = SkillManagerRef->GetObtainableSkillMap();
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
	bIsDisplayingSkillListSet = true;
}

void UCraftingManagerComponent::KeepSkill(int32 SkillID)
{
	if (!SkillManagerRef.IsValid()) return;
	SkillManagerRef->KeepSkillList.Add(SkillID);
}

void UCraftingManagerComponent::UnkeepSkill(int32 SkillID)
{
	if (!SkillManagerRef.IsValid()) return;
	SkillManagerRef->KeepSkillList.Remove(SkillID);
}

bool UCraftingManagerComponent::UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	//��� ������� Ȯ��
	UpdateSkillUpgradeRequiredItemList(SkillID, UpgradeTarget, NewLevel);
	if(!GetIsItemEnough())return false;

	//��ų �߰�
	bool bIsAddSkillSucceed = SkillManagerRef->UpgradeSkill(SkillID, UpgradeTarget, NewLevel);
	if (!bIsAddSkillSucceed) return false;
	else
	{
		//��� �Ҹ�
		ConsumeItem();
		return true;
	}
}

TMap<int32, int32> UCraftingManagerComponent::GetSkillCraftMaterialInfo(int32 SkillID)
{
	if (!SkillManagerRef.IsValid()) return TMap<int32, int32>();
	return SkillManagerRef.Get()->GetSkillInfo(SkillID).CraftMaterial;
}

TMap<int32, int32> UCraftingManagerComponent::UpdateSkillUpgradeRequiredItemList(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	if (!SkillManagerRef.IsValid()) return TMap<int32, int32>();
	if (MainCharacterRef->GetCharacterStat().bInfiniteSkillMaterial && NewLevel == 1) return TMap<int32, int32>();
	RequiredItemInfo.Empty();
	
	uint8 currSkillLevel = 0;

	if(NewLevel > 1)
	{
		currSkillLevel = SkillManagerRef->GetCurrentSkillLevelInfo(SkillID, UpgradeTarget).CurrentLevel + 1;
	}

	for (int32 tempLevel = currSkillLevel + 1; tempLevel <= NewLevel; tempLevel++)
	{
		TMap<int32, int32> tempItemInfo = SkillManagerRef->GetSkillUpgradeLevelInfo(SkillID, UpgradeTarget, tempLevel).RequiredMaterialMap;
		TArray<int32> idList;
		tempItemInfo.GenerateKeyArray(idList);

		for (int32 &id : idList)
		{
			if (!RequiredItemInfo.Contains(id))
				RequiredItemInfo.Add(id, tempItemInfo[id]);
			else
				RequiredItemInfo[id] += tempItemInfo[id];
		}
	} 
	return RequiredItemInfo;
}

TMap<int32, int32> UCraftingManagerComponent::UpdateSkillCraftRequiredItemList(int32 SkillID)
{
	if (!SkillManagerRef.IsValid()) return TMap<int32, int32>();
	
	RequiredItemInfo = SkillManagerRef->GetSkillInfo(SkillID).CraftMaterial;

	return RequiredItemInfo;
}

bool UCraftingManagerComponent::UpgradeWeapon(TArray<uint8> NewLevelList)
{
	//��� ������� Ȯ��
	UpdateWeaponUpgradeRequiredItemList(NewLevelList);
	if (!GetIsItemEnough())return false;
	if(!GetIsStatLevelValid(NewLevelList))return false;

	//��� �Ҹ�
	ConsumeItem();

	return MainCharacterRef->WeaponComponent->UpgradeStat(NewLevelList);
}

TArray<uint8> UCraftingManagerComponent::UpdateWeaponUpgradeRequiredItemList(TArray<uint8> NewLevelList)
{
	return {};
	/* 241004 ���� ���� ����
	RequiredItemInfo.Empty();
	RequiredItemInfo = {0,0,0};
	UWeaponComponentBase* weaponRef = MainCharacterRef->WeaponComponent;
	TArray<uint8> currLevelList;
	TArray<EWeaponStatType> statMapKeyList;
	weaponRef->WeaponState.UpgradedStatMap.GetKeys(statMapKeyList);
	for (EWeaponStatType key : statMapKeyList)
	{
		currLevelList.Add(weaponRef->WeaponState.UpgradedStatMap[key]);
	}

	for (uint8 statType = 0; statType < MAX_WEAPON_UPGRADABLE_STAT_IDX; statType++)
	{
		if (NewLevelList[statType] > currLevelList[statType])
		{
			for (uint8 itemIdx = 0; itemIdx < MAX_CRAFTING_ITEM_IDX; itemIdx++)
			{
				for (uint8 tempLevel = currLevelList[statType] + 1; tempLevel <= NewLevelList[statType]; tempLevel++)
				{
					RequiredItemList[statType] += weaponRef->WeaponStat.UpgradableStatInfoMap[static_cast<EWeaponStatType>(statType + 1)].StatInfoByLevel[tempLevel].RequiredMaterial[itemIdx];
				}
			}
		}
	}
	return RequiredItemList;
	*/
}

bool UCraftingManagerComponent::GetIsStatLevelValid(TArray<uint8> NewLevelList)
{
	UWeaponComponentBase* weaponRef = MainCharacterRef->WeaponComponent;
	for (int32 typeIdx = 0; typeIdx < NewLevelList.Num(); typeIdx++)
	{
		if(weaponRef->GetLimitedStatLevel(static_cast<EWeaponStatType>(typeIdx +1))<NewLevelList[typeIdx]) return false;
	}
	return true;
}
