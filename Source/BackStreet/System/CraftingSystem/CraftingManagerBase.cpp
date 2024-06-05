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
	static ConstructorHelpers::FObjectFinder<UDataTable> skillUpgradeInfoTableFinder(TEXT("/Game/System/CraftingManager/Data/D_SkillUpgradeInfo.D_SkillUpgradeInfo"));
	static ConstructorHelpers::FObjectFinder<UDataTable> playerActiveSkillTableFinder(TEXT("/Game/System/CraftingManager/Data/D_PlayerActiveSkill.D_PlayerActiveSkill"));
	checkf(skillUpgradeInfoTableFinder.Succeeded(), TEXT("SkillUpgradeInfoTable class discovery failed."));
	checkf(playerActiveSkillTableFinder.Succeeded(), TEXT("PlayerActiveSkillTable class discovery failed."));
	SkillUpgradeInfoTable = skillUpgradeInfoTableFinder.Object;
	PlayerActiveSkillTable = playerActiveSkillTableFinder.Object;
}

void UCraftingManagerBase::InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
	checkf(IsValid(GamemodeRef.Get()), TEXT("Failed to get GamemodeRef"));
	
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

bool UCraftingManagerBase::AddSkill(int32 NewSkillID)
{
	if(MainCharacterRef->GetCurrentWeaponRef()->GetWeaponStat().WeaponID == 0) return false;
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
	return true;
}

bool UCraftingManagerBase::UpgradeSkill(int32 NewSkillID, uint8 TempLevel)
{
	ESkillType skillType = GetSkillTypeByID(NewSkillID);
	if(TempLevel == SkillManagerRef->GetCurrSkillLevelByType(skillType)) return false;
	if (!IsSkillUpgradeAvailable(NewSkillID, TempLevel)) return false;
	if (!AddSkill(NewSkillID)) return false;
	FWeaponStateStruct weaponState = MainCharacterRef->GetCurrentWeaponRef()->GetWeaponState();
	FOwnerSkillInfoStruct ownerSkillInfo = weaponState.SkillInfoMap[skillType];
	ownerSkillInfo.SkillLevel = TempLevel;
	weaponState.SkillInfoMap.Add(skillType, ownerSkillInfo);
	
	//하드코딩 BIC이후 제거
	for (int i = 0; i < 3; i++)
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
	checkf(IsValid(SkillUpgradeInfoTable), TEXT("Failed to get CraftinSkillTable"));

	FString rowName = FString::FromInt(NewSkillID);
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	
	return skillUpgradeInfo->MaxLevel;
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
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	if (TempSkillLevel <= currSkillLevel)
	{
		requiredItemAmountList.Init(0, 3);
		return requiredItemAmountList;
	}
	for (int32 itemID = 1; itemID <= 3; itemID++)
	{
		uint8 itemAmount = 0;
		//Map에 해당 재료가 존재하는지 확인
		if (!skillUpgradeInfo->RequiredMaterialMap.Contains(itemID))
		{
			itemAmount = 0;
		}
		else
		{
			//재료가 존재하면 TempLevel까지 갯수를 모두 합함
			for (uint8 temp = currSkillLevel + 1; temp <= TempSkillLevel; temp++)
			{
				itemAmount += skillUpgradeInfo->RequiredMaterialMap[itemID].RequiredMaterialByLevel[temp];
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
	FString rowName = FString::FromInt(NewSkillID);
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	skillUpgradeInfo->SkillVariableMap.GenerateKeyArray(skillVariableKeyList);
	return skillVariableKeyList;
}

TArray<FVariableByLevelStruct> UCraftingManagerBase::GetSkillVariableValueList(int32 NewSkillID)
{
	TArray<FVariableByLevelStruct> skillVariableValueList;
	FString rowName = FString::FromInt(NewSkillID);
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	skillUpgradeInfo->SkillVariableMap.GenerateValueArray(skillVariableValueList);
	return skillVariableValueList;
}