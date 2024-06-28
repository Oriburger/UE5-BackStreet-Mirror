// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerBase.h"
#include "../SkillSystem/SkillManagerBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Item/Weapon/WeaponBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
UCraftingManagerBase::UCraftingManagerBase()
{
}

void UCraftingManagerBase::InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
	checkf(IsValid(GamemodeRef.Get()), TEXT("Failed to get GamemodeRef"));
	
	PlayerActiveSkillTable = GamemodeRef.Get()->PlayerActiveSkillTable;
	StatUpgradeInfoTable = GamemodeRef.Get()->StatUpgradeInfoTable;

	SkillManagerRef = GamemodeRef->GetGlobalSkillManagerBaseRef();
	checkf(IsValid(SkillManagerRef.Get()), TEXT("Failed to get SkillManagerRef"));

	MainCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!IsValid(MainCharacterRef.Get()))
	{
		OnFailedToGetCharacter.Broadcast();
		return;
	}
	TWeakObjectPtr<class AWeaponBase> weaponRef = MainCharacterRef->GetCurrentWeaponRef();
	if (!IsValid(weaponRef.Get())) return;
	{
		OnFailedToGetWeapon.Broadcast();
		return;
	}

	TWeakObjectPtr<class UItemInventoryComponent> itemInventoryRef = MainCharacterRef->ItemInventory;
	if (!IsValid(itemInventoryRef.Get())) return;
	{
		OnFailedToGetItemInventory.Broadcast();
		return;
	}
}

void UCraftingManagerBase::AddSkill(int32 NewSkillID)
{
	if(MainCharacterRef->GetCurrentWeaponRef()->GetWeaponStat().WeaponID == 0) return;
	ESkillType newSkillType = GetSkillTypeByID(NewSkillID);
	FWeaponStateStruct weaponState = MainCharacterRef->GetCurrentWeaponRef()->GetWeaponState();
	//플레이어가 해당 스킬을 가지고 있지 않은경우
	if (!SkillManagerRef->IsSkillInfoStructValid(SkillManagerRef->GetCurrSkillInfoByType(newSkillType)))
	{
		FString rowName = FString::FromInt(NewSkillID);
		FOwnerSkillInfoStruct* ownerSkillInfo = PlayerActiveSkillTable->FindRow<FOwnerSkillInfoStruct>(FName(rowName), rowName);
		checkf(ownerSkillInfo != nullptr, TEXT("PlayerActiveSkillTable Data is not available"));
		weaponState.SkillInfoMap.Add(ownerSkillInfo->SkillType, *ownerSkillInfo);
	}
	//플레이어가 해당 스킬을 가지고 있는 경우
	else
	{
		weaponState.SkillInfoMap[newSkillType].SkillLevel = SkillManagerRef->GetCurrSkillLevelByType(newSkillType);
	}
	MainCharacterRef->GetCurrentWeaponRef()->SetWeaponState(weaponState);
	return;
}

bool UCraftingManagerBase::UpgradeSkill(int32 NewSkillID, uint8 TempLevel)
{
	ESkillType skillType = GetSkillTypeByID(NewSkillID);
	if(TempLevel == SkillManagerRef->GetCurrSkillLevelByType(skillType)) return false;
	if (!IsSkillUpgradeAvailable(NewSkillID, TempLevel)) return false;
	AddSkill(NewSkillID);
	FWeaponStateStruct weaponState = MainCharacterRef->GetCurrentWeaponRef()->GetWeaponState();
	FOwnerSkillInfoStruct ownerSkillInfo = weaponState.SkillInfoMap[skillType];
	ownerSkillInfo.SkillLevel = TempLevel;
	weaponState.SkillInfoMap.Add(skillType, ownerSkillInfo);
	
	//하드코딩 BIC이후 제거
	for (int32 i = 0; i < 3; i++)
	{
		MainCharacterRef->ItemInventory->RemoveItem(i+1, GetRequiredMaterialAmount(SkillManagerRef->GetCurrSkillInfoByType(skillType), TempLevel)[i]);
	}

	MainCharacterRef->GetCurrentWeaponRef()->SetWeaponState(weaponState);
	OnSkillLevelUpdated.Broadcast();
	return true;
}

bool UCraftingManagerBase::IsSkillUpgradeAvailable(int32 NewSkillID, uint8 TempLevel)
{
	FSkillInfoStruct newSkillInfo = SkillManagerRef->GetSkillInfoStructBySkillID(NewSkillID);
	//스킬의 레벨이 최대 업그레이드 가능 수치를 넘지는 않는지 확인
	if(!IsValidLevelForSkillUpgrade(newSkillInfo, TempLevel)) return false;
	//스킬의 업그레이드에 사용될 재료가 충분한지 확인
	if(!IsOwnMaterialEnoughForSkillUpgrade(newSkillInfo, TempLevel)) return false;

	return true;
}

bool UCraftingManagerBase::IsValidLevelForSkillUpgrade(FSkillInfoStruct NewSkillInfo, uint8 TempLevel)
{
	//비교할 대상들이 유효한지 확인
	uint8 maxLevel = GetSkillMaxLevel(NewSkillInfo.SkillID);
	checkf( maxLevel != 0, TEXT("maxLevel is not available"));

	//maxLevel을 초과하는지 확인
	if(TempLevel > maxLevel) return false;
	return true;
}

uint8 UCraftingManagerBase::GetSkillMaxLevel(int32 NewSkillID)
{
	FSkillUpgradeInfoStruct skillUpgradeInfo = GetSkillUpgradeInfoStructBySkillID(NewSkillID);
	return skillUpgradeInfo.MaxLevel;
}

bool UCraftingManagerBase::IsOwnMaterialEnoughForSkillUpgrade(FSkillInfoStruct NewSkillInfo, uint8 TempLevel)
{
	//비교할 대상들이 유효한지 확인
	if (!SkillManagerRef->IsSkillInfoStructValid(NewSkillInfo)) return false;
	TArray<uint8> currItemAmountList = MainCharacterRef->ItemInventory->GetCraftingItemAmount();
	TArray<uint8> requiredItemAmountList = GetRequiredMaterialAmount(NewSkillInfo, TempLevel);

	//하드코딩 BIC이후 변경
	for (int32 itemID = 1; itemID <= 3; itemID++)
	{
		//현재 지닌 재료의 갯수가 요구량보다 많은지 확인
		if (currItemAmountList[itemID-1]<requiredItemAmountList[itemID-1]) return false;
	}
	return true;
}

TArray<uint8> UCraftingManagerBase::GetRequiredMaterialAmount(FSkillInfoStruct NewSkillInfo, uint8 TempSkillLevel)
{
	TArray<uint8> requiredItemAmountList;
	uint8 currSkillLevel = 0;
	ESkillType skillType = NewSkillInfo.SkillType;
	if (SkillManagerRef->IsSkillInfoStructValid(SkillManagerRef->GetCurrSkillInfoByType(skillType)))
	{
		currSkillLevel = SkillManagerRef->GetCurrSkillLevelByType(skillType);
	}
	FString rowName = FString::FromInt(NewSkillInfo.SkillID);
	FSkillUpgradeInfoStruct skillUpgradeInfo = GetSkillUpgradeInfoStructBySkillID(NewSkillInfo.SkillID);
	if (TempSkillLevel <= currSkillLevel)
	{
		requiredItemAmountList.Init(0, 3);
		return requiredItemAmountList;
	}
	for (int32 itemID = 1; itemID <= 3; itemID++)
	{
		uint8 itemAmount = 0;
		//Map에 해당 재료가 존재하는지 확인
		if (!skillUpgradeInfo.RequiredMaterialMap.Contains(itemID))
		{
			itemAmount = 0;
		}
		else
		{
			//재료가 존재하면 TempLevel까지 갯수를 모두 합함
			for (uint8 temp = currSkillLevel + 1; temp <= TempSkillLevel; temp++)
			{
				itemAmount += skillUpgradeInfo.RequiredMaterialMap[itemID].RequiredMaterialByLevel[temp];
			}
		}
		//요구 재료 배열에 추가함
		requiredItemAmountList.Add(itemAmount);
	}
	return requiredItemAmountList;
}

ESkillType UCraftingManagerBase::GetSkillTypeByID(int32 NewSkillID)
{
	FString rowName = FString::FromInt(NewSkillID);
	FOwnerSkillInfoStruct* ownerSkillInfo = PlayerActiveSkillTable->FindRow<FOwnerSkillInfoStruct>(FName(rowName), rowName);
	checkf(ownerSkillInfo != nullptr, TEXT("PlayerActiveSkillTable Data is not available"));
	return ownerSkillInfo->SkillType;
}

TArray<FName> UCraftingManagerBase::GetSkillVariableKeyList(int32 NewSkillID)
{
	TArray<FName> skillVariableKeyList;
	FSkillUpgradeInfoStruct skillUpgradeInfo = GetSkillUpgradeInfoStructBySkillID(NewSkillID);
	skillUpgradeInfo.SkillVariableMap.GenerateKeyArray(skillVariableKeyList);
	return skillVariableKeyList;
}

TArray<FVariableByLevelStruct> UCraftingManagerBase::GetSkillVariableValueList(int32 NewSkillID)
{
	TArray<FVariableByLevelStruct> skillVariableValueList;
	FSkillUpgradeInfoStruct skillUpgradeInfo = GetSkillUpgradeInfoStructBySkillID(NewSkillID);
	skillUpgradeInfo.SkillVariableMap.GenerateValueArray(skillVariableValueList);
	return skillVariableValueList;
}

FSkillUpgradeInfoStruct UCraftingManagerBase::GetSkillUpgradeInfoStructBySkillID(int32 SkillID)
{
	if (!GamemodeRef.IsValid() || !IsValid(GamemodeRef.Get()->GetSkillManagerRef())) return FSkillUpgradeInfoStruct();
	return GamemodeRef.Get()->GetSkillManagerRef()->GetSkillUpgradeInfoStructBySkillID(SkillID);
}

bool UCraftingManagerBase::UpgradeStat(TArray<uint8> TempUpgradedStatList)
{
	if (!IsStatUpgradeAvailable(TempUpgradedStatList)) return false;
	FWeaponStateStruct weaponState = MainCharacterRef->GetCurrentWeaponRef()->GetWeaponState();
	weaponState.UpgradedStatList[0] = TempUpgradedStatList[0];
	weaponState.UpgradedStatList[1] = TempUpgradedStatList[1];
	weaponState.UpgradedStatList[2] = TempUpgradedStatList[2];

	int32 currWeaponID = MainCharacterRef->GetCurrentWeaponRef()->WeaponID;
	FString rowName = FString::FromInt(currWeaponID);
	FStatUpgradeInfoStruct* statUpgradeInfo = StatUpgradeInfoTable->FindRow<FStatUpgradeInfoStruct>(FName(rowName), rowName);

	FWeaponStatStruct weaponStat = MainCharacterRef->GetCurrentWeaponRef()->GetWeaponStat();
	weaponStat.WeaponDamage = statUpgradeInfo->RequiredInfo[0].RequiredMaterialByLevel[TempUpgradedStatList[0]].StatByLevel;
	weaponStat.WeaponAtkSpeedRate = statUpgradeInfo->RequiredInfo[0].RequiredMaterialByLevel[TempUpgradedStatList[1]].StatByLevel;
	//weaponStat.파이널 임팩트 = statUpgradeInfo->RequiredInfo[0].RequiredMaterialByLevel[TempUpgradedStatList[2]].StatByLevel
	//하드코딩 BIC이후 제거
	for (int i = 0; i < 3; i++)
	{
		MainCharacterRef->ItemInventory->RemoveItem(i + 1, GetRequiredMaterialAmountForStat(TempUpgradedStatList)[i]);
	}

	MainCharacterRef->GetCurrentWeaponRef()->SetWeaponStat(weaponStat);
	MainCharacterRef->GetCurrentWeaponRef()->SetWeaponState(weaponState);
	UE_LOG(LogTemp, Log, TEXT("Damage : %f"), MainCharacterRef->GetCurrentWeaponRef()->GetWeaponStat().WeaponDamage);
	UE_LOG(LogTemp, Log, TEXT("Atk : %f"), MainCharacterRef->GetCurrentWeaponRef()->GetWeaponStat().WeaponAtkSpeedRate);
	OnStatLevelUpdated.Broadcast();
	return true;
}

bool UCraftingManagerBase::IsStatUpgradeAvailable(TArray<uint8> TempUpgradedStatList)
{
	if (!IsValidLevelForStatUpgrade(TempUpgradedStatList)) return false;
	if (!IsOwnMaterialEnoughForStatUpgrade(TempUpgradedStatList)) return false;

	return true;
}

bool UCraftingManagerBase::IsValidLevelForStatUpgrade(TArray<uint8> TempUpgradedStatList)
{
	//무기의 현재레벨이 Stat의 max 수치보다 작은지 확인
	TArray<uint8> maxLevelList = GetStatMaxLevel();
	for (uint8 idx = 0; idx < 3; idx++)
	{
		if (TempUpgradedStatList[idx] > maxLevelList[idx]) return false;
	}
	return true;
}

bool UCraftingManagerBase::IsOwnMaterialEnoughForStatUpgrade(TArray<uint8> TempUpgradedStatList)
{
	TArray<uint8> currItemAmountList = MainCharacterRef->ItemInventory->GetCraftingItemAmount();
	TArray<uint8> requiredItemAmountList = GetRequiredMaterialAmountForStat(TempUpgradedStatList);

	//하드코딩 BIC이후 변경
	for (int32 itemID = 1; itemID <= 3; itemID++)
	{
		//현재 지닌 재료의 갯수가 요구량보다 많은지 확인
		if (currItemAmountList[itemID - 1] < requiredItemAmountList[itemID - 1]) return false;
	}
	return true;
}

TArray<uint8> UCraftingManagerBase::GetRequiredMaterialAmountForStat(TArray<uint8> TempUpgradedStatList)
{
	int32 currWeaponID = MainCharacterRef->GetCurrentWeaponRef()->WeaponStat.WeaponID;
	TArray<uint8> currLevel = GetCurrentStatLevel();
	TArray<uint8> requiredMaterialList = {0, 0, 0};
	FString rowName = FString::FromInt(currWeaponID);
	FStatUpgradeInfoStruct* statUpgradeInfo = StatUpgradeInfoTable->FindRow<FStatUpgradeInfoStruct>(FName(rowName), rowName);
	//스탯구분
	for (uint8 idx = 0; idx < 3; idx++)
	{
		//현재 레벨로부터의 합산
		for (uint8 calcLevel = currLevel[idx] + 1; calcLevel <= TempUpgradedStatList[idx]; calcLevel++)
		{
			//재료
			for (uint8 idx2 = 0; idx2< 3; idx2++)
			{
				requiredMaterialList[idx2] += statUpgradeInfo->RequiredInfo[idx].RequiredMaterialByLevel[calcLevel].RequiredMaterial[idx2];
			}
		}
	}
	return requiredMaterialList;
}

TArray<uint8> UCraftingManagerBase::GetCurrentStatLevel()
{
	return MainCharacterRef->GetCurrentWeaponRef()->GetWeaponState().UpgradedStatList;
}

TArray<uint8> UCraftingManagerBase::GetStatMaxLevel()
{
	int32 currWeaponID = MainCharacterRef->GetCurrentWeaponRef()->WeaponID;
	FString rowName = FString::FromInt(currWeaponID);
	FStatUpgradeInfoStruct* statUpgradeInfo = StatUpgradeInfoTable->FindRow<FStatUpgradeInfoStruct>(FName(rowName), rowName);
	TArray<uint8> maxLevelList;
	for (uint8 idx = 0; idx < 3; idx++)
	{
		maxLevelList.Add(statUpgradeInfo->RequiredInfo[idx].MaxLevel);
	}
	return maxLevelList;
}
