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

bool UCraftingManagerBase::AddSkill(int32 SkillID)
{
	FString rowName = FString::FromInt(SkillID);
	FOwnerSkillInfoStruct* ownerSkillInfo = PlayerActiveSkillTable->FindRow<FOwnerSkillInfoStruct>(FName(rowName), rowName);
	checkf(ownerSkillInfo == nullptr, TEXT("PlayerActiveSkillTable Data is not available"));
	FWeaponStateStruct weaponState= MainCharacterRef->GetCurrentWeaponRef()->GetWeaponState();
	weaponState.SkillInfoMap.Add(ownerSkillInfo->SkillType, *ownerSkillInfo);
	MainCharacterRef->GetCurrentWeaponRef()->SetWeaponState(weaponState);
	return true;
}

bool UCraftingManagerBase::UpgradeSkill(ESkillType SkillType, uint8 Adder)
{
	if(Adder == 0) return false;
	if (!IsSkillUpgradeAvailable(SkillType, Adder)) return false;
	FWeaponStateStruct weaponState = MainCharacterRef->GetCurrentWeaponRef()->GetWeaponState();
	FOwnerSkillInfoStruct ownerSkillInfo = weaponState.SkillInfoMap[SkillType];
	ownerSkillInfo.SkillLevel += Adder;
	weaponState.SkillInfoMap.Add(SkillType, ownerSkillInfo);

	MainCharacterRef->GetCurrentWeaponRef()->SetWeaponState(weaponState);
	return true;
}

bool UCraftingManagerBase::IsSkillUpgradeAvailable(ESkillType SkillType, uint8 Adder)
{
	//캐릭터가 현재 가진 스킬(스킬 타입에 따른)이 유효한지 확인
	FSkillInfoStruct skillInfo = SkillManagerRef->GetCurrentSkillInfoByType(SkillType);
	if(!SkillManagerRef->IsSkillInfoStructValid(skillInfo)) return false;
	//스킬의 레벨이 최대 업그레이드 가능 수치를 넘지는 않는지 확인
	if(!IsValidLevelForSkillUpgrade(skillInfo, Adder)) return false;
	//스킬의 업그레이드에 사용될 재료가 충분한지 확인
	if(!IsOwnMaterialEnoughForSkillUpgrade(skillInfo, Adder)) return false;

	return true;
}

bool UCraftingManagerBase::IsValidLevelForSkillUpgrade(FSkillInfoStruct SkillInfo, uint8 Adder)
{
	//비교할 대상들이 유효한지 확인
	if (!SkillManagerRef->IsSkillInfoStructValid(SkillInfo)) return false;
	uint8 maxLevel = GetSkillMaxLevel(SkillInfo.SkillID);
	checkf( maxLevel == 0, TEXT("maxLevel is not available"));

	//maxLevel을 초과하는지 확인
	if(SkillInfo.SkillLevelStruct.SkillLevel + Adder > maxLevel) return false;
	return true;
}

uint8 UCraftingManagerBase::GetSkillMaxLevel(int32 SkillID)
{
	checkf(IsValid(SkillUpgradeInfoTable), TEXT("Failed to get CraftinSkillTable"));

	FString rowName = FString::FromInt(SkillID);
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	
	return skillUpgradeInfo->MaxLevel;
}

bool UCraftingManagerBase::IsOwnMaterialEnoughForSkillUpgrade(FSkillInfoStruct SkillInfo, uint8 Adder)
{
	//비교할 대상들이 유효한지 확인
	if (!SkillManagerRef->IsSkillInfoStructValid(SkillInfo)) return false;
	TArray<uint8> currItemAmountList = MainCharacterRef->ItemInventory->GetCraftingItemAmount();
	TArray<uint8> requiredItemAmountList = GetRequiredMaterialAmount(SkillInfo, Adder);

	for (int32 itemID = 1; itemID <= 3; itemID++)
	{
		//현재 지닌 재료의 갯수가 요구량보다 많은지 확인
		if (currItemAmountList[itemID]<requiredItemAmountList[itemID]) return false;
	}
	return true;
}

TArray<uint8> UCraftingManagerBase::GetRequiredMaterialAmount(FSkillInfoStruct SkillInfo, uint8 Adder)
{
	TArray<uint8> requiredItemAmountList;
	FString rowName = FString::FromInt(SkillInfo.SkillID);
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	for (int32 itemID = 1; itemID <= 3; itemID++)
	{
		uint8 itemAmount = 0;
		for (uint8 temp = 1; temp <= Adder; temp++)
		{
			itemAmount += skillUpgradeInfo->RequiredMaterialMap[itemID].RequiredMaterialByLevel[SkillInfo.SkillLevelStruct.SkillLevel+temp];
		}
			requiredItemAmountList.Add(itemAmount);
	}
	return requiredItemAmountList;
}