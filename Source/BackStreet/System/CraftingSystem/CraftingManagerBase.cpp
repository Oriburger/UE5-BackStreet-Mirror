// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
UCraftingManagerBase::UCraftingManagerBase()
{
}

//void UCraftingManagerBase::InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef)
//{
//	if (!IsValid(NewGamemodeRef)) return;
//	GamemodeRef = NewGamemodeRef;
//	checkf(IsValid(GamemodeRef.Get()), TEXT("Failed to get GamemodeRef"));
//	
//	PlayerActiveSkillTable = GamemodeRef.Get()->PlayerActiveSkillTable;
//	StatUpgradeInfoTable = GamemodeRef.Get()->StatUpgradeInfoTable;
//
//
//	MainCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
//	if (!IsValid(MainCharacterRef.Get()))
//	{
//		OnFailedToGetCharacter.Broadcast();
//		return;
//	}
//
//	TWeakObjectPtr<class UItemInventoryComponent> itemInventoryRef = MainCharacterRef->ItemInventory;
//	if (!IsValid(itemInventoryRef.Get())) return;
//	{
//		OnFailedToGetItemInventory.Broadcast();
//		return;
//	}
//}

//void UCraftingManagerBase::AddSkill(int32 NewSkillID)
//{
//	/*
//	if(MainCharacterRef->WeaponComponent->GetWeaponStat().WeaponID == 0) return;
//	ESkillType newSkillType = GetSkillTypeByID(NewSkillID);
//	FWeaponStateStruct weaponState = MainCharacterRef->WeaponComponent->GetWeaponState();
//	//�÷��̾ �ش� ��ų�� ������ ���� �������
//	if (!SkillManagerRef->IsSkillInfoStructValid(SkillManagerRef->GetCurrSkillInfoByType(newSkillType)))
//	{
//		FString rowName = FString::FromInt(NewSkillID);
//		FOwnerSkillInfoStruct* ownerSkillInfo = PlayerActiveSkillTable->FindRow<FOwnerSkillInfoStruct>(FName(rowName), rowName);
//		checkf(ownerSkillInfo != nullptr, TEXT("PlayerActiveSkillTable Data is not available"));
//		weaponState.SkillInfoMap.Add(ownerSkillInfo->SkillType, *ownerSkillInfo);
//	}
//	//�÷��̾ �ش� ��ų�� ������ �ִ� ���
//	else
//	{
//		weaponState.SkillInfoMap[newSkillType].SkillLevel = SkillManagerRef->GetCurrSkillLevelByType(newSkillType);
//	}
//	MainCharacterRef->WeaponComponent->SetWeaponState(weaponState);
//	return;
//	*/
//}
//
//bool UCraftingManagerBase::UpgradeSkill(int32 NewSkillID, uint8 TempLevel)
//{
//	/*
//	ESkillType skillType = GetSkillTypeByID(NewSkillID);
//	if(TempLevel == SkillManagerRef->GetCurrSkillLevelByType(skillType)) return false;
//	if (!IsSkillUpgradeAvailable(NewSkillID, TempLevel)) return false;
//	AddSkill(NewSkillID);
//	FWeaponStateStruct weaponState = MainCharacterRef->WeaponComponent->GetWeaponState();
//	if (!weaponState.SkillInfoMap.Contains(skillType)) return false;
//	FOwnerSkillInfoStruct ownerSkillInfo = weaponState.SkillInfoMap[skillType];
//	ownerSkillInfo.SkillLevel = TempLevel;
//	weaponState.SkillInfoMap.Add(skillType, ownerSkillInfo);
//	
//	//�ϵ��ڵ� BIC���� ����
//	for (int32 i = 0; i < 3; i++)
//	{
//		MainCharacterRef->ItemInventory->RemoveItem(i+1, GetRequiredMaterialAmount(SkillManagerRef->GetCurrSkillInfoByType(skillType), TempLevel)[i]);
//	}
//
//	MainCharacterRef->WeaponComponent->SetWeaponState(weaponState);
//	OnSkillLevelUpdated.Broadcast();
//	return true;
//	*/
//	return bool();
//}
//
//bool UCraftingManagerBase::IsSkillUpgradeAvailable(int32 NewSkillID, uint8 TempLevel)
//{
//	/*
//	FSkillInfoStruct newSkillInfo = SkillManagerRef->GetSkillInfoStructBySkillID(NewSkillID);
//	//��ų�� ������ �ִ� ���׷��̵� ���� ��ġ�� ������ �ʴ��� Ȯ��
//	if(!IsValidLevelForSkillUpgrade(newSkillInfo, TempLevel)) return false;
//	//��ų�� ���׷��̵忡 ���� ��ᰡ ������� Ȯ��
//	if(!IsOwnMaterialEnoughForSkillUpgrade(newSkillInfo, TempLevel)) return false;
//
//	return true;
//	*/
//	return bool();
//}
//
//bool UCraftingManagerBase::IsValidLevelForSkillUpgrade(FSkillInfoStruct NewSkillInfo, uint8 TempLevel)
//{
//	////���� ������ ��ȿ���� Ȯ��
//	//uint8 maxlevel = getskillmaxlevel(newskillinfo.skillid);
//	//checkf( maxlevel != 0, text("maxlevel is not available"));
//
//	////maxlevel�� �ʰ��ϴ��� Ȯ��
//	//if(templevel > maxlevel) return false;
//	//return true;
//	return bool();
//}
//
//uint8 UCraftingManagerBase::GetSkillMaxLevel(int32 NewSkillID)
//{
//	//FSkillUpgradeInfoStruct skillUpgradeInfo = GetSkillUpgradeInfoStructBySkillID(NewSkillID);
//	//return skillUpgradeInfo.MaxLevel;
//	return  uint8();
//}
//
//bool UCraftingManagerBase::IsOwnMaterialEnoughForSkillUpgrade(FSkillInfoStruct NewSkillInfo, uint8 TempLevel)
//{
//	////���� ������ ��ȿ���� Ȯ��
//	//if (!SkillManagerRef->IsSkillInfoStructValid(NewSkillInfo)) return false;
//	//TArray<uint8> currItemAmountList = MainCharacterRef->ItemInventory->GetCraftingItemAmount();
//	//TArray<uint8> requiredItemAmountList = GetRequiredMaterialAmount(NewSkillInfo, TempLevel);
//
//	////�ϵ��ڵ� BIC���� ����
//	//for (int32 itemID = 1; itemID <= 3; itemID++)
//	//{
//	//	//���� ���� ����� ������ �䱸������ ������ Ȯ��
//	//	if (currItemAmountList[itemID-1]<requiredItemAmountList[itemID-1]) return false;
//	//}
//	//return true;
//	return bool();
//}
//
//TArray<uint8> UCraftingManagerBase::GetRequiredMaterialAmount(FSkillInfoStruct NewSkillInfo, uint8 TempSkillLevel)
//{
//	//TArray<uint8> requiredItemAmountList;
//	//uint8 currSkillLevel = 0;
//	//ESkillType skillType = NewSkillInfo.SkillType;
//	//if (SkillManagerRef->IsSkillInfoStructValid(SkillManagerRef->GetCurrSkillInfoByType(skillType)))
//	//{
//	//	currSkillLevel = SkillManagerRef->GetCurrSkillLevelByType(skillType);
//	//}
//	//FString rowName = FString::FromInt(NewSkillInfo.SkillID);
//	//FSkillUpgradeInfoStruct skillUpgradeInfo = GetSkillUpgradeInfoStructBySkillID(NewSkillInfo.SkillID);
//	//if (TempSkillLevel <= currSkillLevel)
//	//{
//	//	requiredItemAmountList.Init(0, 3);
//	//	return requiredItemAmountList;
//	//}
//	//for (int32 itemID = 1; itemID <= 3; itemID++)
//	//{
//	//	uint8 itemAmount = 0;
//	//	//Map�� �ش� ��ᰡ �����ϴ��� Ȯ��
//	//	if (!skillUpgradeInfo.RequiredMaterialMap.Contains(itemID))
//	//	{
//	//		itemAmount = 0;
//	//	}
//	//	else
//	//	{
//	//		//��ᰡ �����ϸ� TempLevel���� ������ ��� ����
//	//		for (uint8 temp = currSkillLevel + 1; temp <= TempSkillLevel; temp++)
//	//		{
//	//			itemAmount += skillUpgradeInfo.RequiredMaterialMap[itemID].RequiredMaterialByLevel[temp];
//	//		}
//	//	}
//	//	//�䱸 ��� �迭�� �߰���
//	//	requiredItemAmountList.Add(itemAmount);
//	//}
//	//return requiredItemAmountList;
//	return TArray<uint8>();
//}
//
//ESkillType UCraftingManagerBase::GetSkillTypeByID(int32 NewSkillID)
//{
//	//FString rowName = FString::FromInt(NewSkillID);
//	//FOwnerSkillInfoStruct* ownerSkillInfo = PlayerActiveSkillTable->FindRow<FOwnerSkillInfoStruct>(FName(rowName), rowName);
//	//checkf(ownerSkillInfo != nullptr, TEXT("PlayerActiveSkillTable Data is not available"));
//	//return ownerSkillInfo->SkillType;
//	return ESkillType();
//}
//
//TArray<FName> UCraftingManagerBase::GetSkillVariableKeyList(int32 NewSkillID)
//{
//	//TArray<FName> skillVariableKeyList;
//	//FSkillUpgradeInfoStruct skillUpgradeInfo = GetSkillUpgradeInfoStructBySkillID(NewSkillID);
//	//skillUpgradeInfo.SkillVariableMap.GenerateKeyArray(skillVariableKeyList);
//	//return skillVariableKeyList;
//	return TArray<FName>();
//}
//
//TArray<FVariableByLevelStruct> UCraftingManagerBase::GetSkillVariableValueList(int32 NewSkillID)
//{
//	//TArray<FVariableByLevelStruct> skillVariableValueList;
//	//FSkillUpgradeInfoStruct skillUpgradeInfo = GetSkillUpgradeInfoStructBySkillID(NewSkillID);
//	//skillUpgradeInfo.SkillVariableMap.GenerateValueArray(skillVariableValueList);
//	//return skillVariableValueList;
//	return TArray<FVariableByLevelStruct>();
//}
//
//FSkillUpgradeInfoStruct UCraftingManagerBase::GetSkillUpgradeInfoStructBySkillID(int32 SkillID)
//{
//	//if (!GamemodeRef.IsValid() || !IsValid(GamemodeRef.Get()->GetSkillManagerRef())) return FSkillUpgradeInfoStruct();
//	//return GamemodeRef.Get()->GetSkillManagerRef()->GetSkillUpgradeInfoStructBySkillID(SkillID);
//	return FSkillUpgradeInfoStruct();
//}
//
//bool UCraftingManagerBase::UpgradeStat(TArray<uint8> TempUpgradedStatList)
//{
//	//if (!IsStatUpgradeAvailable(TempUpgradedStatList)) return false;
//	//FWeaponStateStruct weaponState = MainCharacterRef->WeaponComponent->GetWeaponState();
//	//weaponState.UpgradedStatList[0] = TempUpgradedStatList[0];
//	//weaponState.UpgradedStatList[1] = TempUpgradedStatList[1];
//	//weaponState.UpgradedStatList[2] = TempUpgradedStatList[2];
//
//	//int32 currWeaponID = MainCharacterRef->WeaponComponent->WeaponID;
//	//FString rowName = FString::FromInt(currWeaponID);
//	//FStatUpgradeInfoStruct* statUpgradeInfo = StatUpgradeInfoTable->FindRow<FStatUpgradeInfoStruct>(FName(rowName), rowName);
//
//	//FWeaponStatStruct weaponStat = MainCharacterRef->WeaponComponent->GetWeaponStat();
//	//weaponStat.WeaponDamage = statUpgradeInfo->RequiredInfo[0].RequiredMaterialByLevel[TempUpgradedStatList[0]].StatByLevel;
//	//weaponStat.WeaponAtkSpeedRate = statUpgradeInfo->RequiredInfo[0].RequiredMaterialByLevel[TempUpgradedStatList[1]].StatByLevel;
//	//weaponStat.FinalImpactStrength = statUpgradeInfo->RequiredInfo[0].RequiredMaterialByLevel[TempUpgradedStatList[2]].StatByLevel;
//	////�ϵ��ڵ� BIC���� ����
//	//for (int i = 0; i < 3; i++)
//	//{
//	//	MainCharacterRef->ItemInventory->RemoveItem(i + 1, GetRequiredMaterialAmountForStat(TempUpgradedStatList)[i]);
//	//}
//
//	//MainCharacterRef->WeaponComponent->SetWeaponStat(weaponStat);
//	//MainCharacterRef->WeaponComponent->SetWeaponState(weaponState);
//	//UE_LOG(LogTemp, Log, TEXT("Damage : %f"), MainCharacterRef->WeaponComponent->GetWeaponStat().WeaponDamage);
//	//UE_LOG(LogTemp, Log, TEXT("Atk : %f"), MainCharacterRef->WeaponComponent->GetWeaponStat().WeaponAtkSpeedRate);
//	//OnStatLevelUpdated.Broadcast();
//	//return true;
//	return bool();
//}
//
//bool UCraftingManagerBase::IsStatUpgradeAvailable(TArray<uint8> TempUpgradedStatList)
//{
//	//if (!IsValidLevelForStatUpgrade(TempUpgradedStatList)) return false;
//	//if (!IsOwnMaterialEnoughForStatUpgrade(TempUpgradedStatList)) return false;
//
//	//return true;
//	return bool();
//}
//
//bool UCraftingManagerBase::IsValidLevelForStatUpgrade(TArray<uint8> TempUpgradedStatList)
//{
//	////������ ���緹���� Stat�� max ��ġ���� ������ Ȯ��
//	//TArray<uint8> maxLevelList = GetStatMaxLevel();
//	//for (uint8 idx = 0; idx < 3; idx++)
//	//{
//	//	if (TempUpgradedStatList[idx] > maxLevelList[idx]) return false;
//	//}
//	//return true;
//	return bool();
//}
//
//bool UCraftingManagerBase::IsOwnMaterialEnoughForStatUpgrade(TArray<uint8> TempUpgradedStatList)
//{
//	//TArray<uint8> currItemAmountList = MainCharacterRef->ItemInventory->GetCraftingItemAmount();
//	//TArray<uint8> requiredItemAmountList = GetRequiredMaterialAmountForStat(TempUpgradedStatList);
//
//	////�ϵ��ڵ� BIC���� ����
//	//for (int32 itemID = 1; itemID <= 3; itemID++)
//	//{
//	//	//���� ���� ����� ������ �䱸������ ������ Ȯ��
//	//	if (currItemAmountList[itemID - 1] < requiredItemAmountList[itemID - 1]) return false;
//	//}
//	//return true;
//	return bool();
//}
//
//TArray<uint8> UCraftingManagerBase::GetRequiredMaterialAmountForStat(TArray<uint8> TempUpgradedStatList)
//{
//	//int32 currWeaponID = MainCharacterRef->WeaponComponent->WeaponStat.WeaponID;
//	//TArray<uint8> currLevel = GetCurrentStatLevel();
//	//TArray<uint8> requiredMaterialList = {0, 0, 0};
//	//FString rowName = FString::FromInt(currWeaponID);
//	//FStatUpgradeInfoStruct* statUpgradeInfo = StatUpgradeInfoTable->FindRow<FStatUpgradeInfoStruct>(FName(rowName), rowName);
//	////���ȱ���
//	//for (uint8 idx = 0; idx < 3; idx++)
//	//{
//	//	//���� �����κ����� �ջ�
//	//	for (uint8 calcLevel = currLevel[idx] + 1; calcLevel <= TempUpgradedStatList[idx]; calcLevel++)
//	//	{
//	//		//���
//	//		for (uint8 idx2 = 0; idx2< 3; idx2++)
//	//		{
//	//			requiredMaterialList[idx2] += statUpgradeInfo->RequiredInfo[idx].RequiredMaterialByLevel[calcLevel].RequiredMaterial[idx2];
//	//		}
//	//	}
//	//}
//	//return requiredMaterialList;
//	return TArray<uint8>();
//}
//
//TArray<uint8> UCraftingManagerBase::GetCurrentStatLevel()
//{
//	//return MainCharacterRef->WeaponComponent->GetWeaponState().UpgradedStatList;
//	return TArray<uint8>();
//}
//
//TArray<uint8> UCraftingManagerBase::GetStatMaxLevel()
//{
//	//int32 currWeaponID = MainCharacterRef->WeaponComponent->WeaponID;
//	//FString rowName = FString::FromInt(currWeaponID);
//	//FStatUpgradeInfoStruct* statUpgradeInfo = StatUpgradeInfoTable->FindRow<FStatUpgradeInfoStruct>(FName(rowName), rowName);
//	//TArray<uint8> maxLevelList;
//	//for (uint8 idx = 0; idx < 3; idx++)
//	//{
//	//	if (statUpgradeInfo->RequiredInfo.IsValidIndex(idx))
//	//	{
//	//		maxLevelList.Add(statUpgradeInfo->RequiredInfo[idx].MaxLevel);
//	//	}
//	//}
//	//return maxLevelList;
//	return TArray<uint8>();
//}
//
//TArray<uint8> UCraftingManagerBase::GetStatMaxLevelByWeaponID(int32 WeaponID)
//{
//	//FString rowName = FString::FromInt(WeaponID);
//	//FStatUpgradeInfoStruct* statUpgradeInfo = StatUpgradeInfoTable->FindRow<FStatUpgradeInfoStruct>(FName(rowName), rowName);
//	//TArray<uint8> maxLevelList;
//	//for (uint8 idx = 0; idx < 3; idx++)
//	//{
//	//	if (statUpgradeInfo->RequiredInfo.IsValidIndex(idx))
//	//	{
//	//		maxLevelList.Add(statUpgradeInfo->RequiredInfo[idx].MaxLevel);
//	//	}
//	//}
//	//return maxLevelList;
//	return TArray<uint8>();
//	*/
//}