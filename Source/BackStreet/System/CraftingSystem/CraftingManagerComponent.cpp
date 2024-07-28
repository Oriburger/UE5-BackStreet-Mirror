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
		//만능재료 사용 시 충족 여부 확인
		if (KeepMatIdx == requiredMatIdx && 
			currMatList[4] < RequiredMatList[requiredMatIdx]) return false;
		//일반재료 사용 시 충족 여부 확인
		if(currMatList[requiredMatIdx] < RequiredMatList[requiredMatIdx]) return false;
	}
	return true;
}

bool UCraftingManagerComponent::ConsumeMat()
{
	for (uint8 idx = 0; idx < MAX_CRAFTING_ITEM_IDX; idx++)
	{
		//만능재료 사용 시
		if (KeepMatIdx == idx)
		{
			MainCharacterRef->ItemInventory->RemoveItem(4, RequiredMatList[idx]);
		}
		//기본재료 사용 시
		else
		{
			MainCharacterRef->ItemInventory->RemoveItem(idx + 1, RequiredMatList[idx]);
		}
	}
	return false;
}

bool UCraftingManagerComponent::AddSkill(int32 NewSkillID)
{	
	//재료 충분한지 확인
	UpdateRequiredMatForSU(NewSkillID, 1);
	if (!IsMatEnough())return false;
	
	//스킬 추가
	bool bIsAddSkillSucceed = SkillManagerRef->AddSkill(NewSkillID);
	if (!bIsAddSkillSucceed) return false;
	else
	{
		//재료 소모
		ConsumeMat();
		//스킬 제작UI에서 노출 되었던 스킬 리스트에 등록
		TArray<int32> displayedSkillIDList;
		DisplayingSkillMap.GenerateValueArray(displayedSkillIDList);
		for (int32 skillID : displayedSkillIDList)
		{
			SkillManagerRef->DisplayedSkillList.Add(skillID);
		}

		//스킬 중복 추가 방지
		bIsSkillCreated = true;
		return true;
	}
}

void UCraftingManagerComponent::SetDisplayingSkillList()
{
	DisplayingSkillMap.Reset();
	TArray<ESkillType> skillTypeList = MainCharacterRef->WeaponComponent->WeaponStat.SkillTypeList;
	//DisplayingSkillMap 초기화
	for (ESkillType skillType : skillTypeList)
	{
		DisplayingSkillMap.Add(skillType,0);
	}

	//찜 되어있는 스킬 타입이 있는지 확인
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

	//찜 안되어 있는 스킬 타입은 랜덤으로 가중치 고려하여 선택
	for (ESkillType skillType : skillTypeList)
	{
		if(DisplayingSkillMap.Find(skillType)!=0) continue;
		else 
		{
			TMap<ESkillType, FObtainableSkillListContainer>obtainableSkillMap = SkillManagerRef->GetObtainableSkillMap();
			checkf(obtainableSkillMap.Contains(skillType), TEXT("ObtainableSkillMap is something wrong"));
			//가중치 랜덤 로직
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
	//재료 충분한지 확인
	UpdateRequiredMatForSU(SkillID, NewLevel);
	if(!IsMatEnough())return false;

	//스킬 추가
	bool bIsAddSkillSucceed = SkillManagerRef->UpgradeSkill(SkillID, NewLevel);
	if (!bIsAddSkillSucceed) return false;
	else
	{
		//재료 소모
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
	//재료 충분한지 확인
	UpdateRequiredMatForWU(NewLevelList);
	if (!IsMatEnough())return false;

	//재료 소모
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