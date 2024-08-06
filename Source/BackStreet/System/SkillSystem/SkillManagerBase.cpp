// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillManagerBase.h"
#include "../SkillSystem/SkillBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/CharacterBase.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USkillManagerBase::USkillManagerBase()
{
}

void USkillManagerBase::InitSkillManagerBase(ABackStreetGameModeBase* NewGamemodeRef)
{
	checkf(IsValid(NewGamemodeRef), TEXT("Failed to get GameMode reference"));
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
	SkillInfoTable = GamemodeRef.Get()->SkillInfoTable;
	SkillUpgradeInfoTable = GamemodeRef.Get()->SkillUpgradeInfoTable;
}

void USkillManagerBase::TrySkill(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	ASkillBase* skillBase = ActivateSkillBase(NewCauser, OwnerSkillInfo); //ambigious name 
	if(!IsValid(skillBase)) return;
	NewCauser->SetActionState(ECharacterActionType::E_Skill);
	skillBase->ActivateSkill();
}

ASkillBase* USkillManagerBase::ActivateSkillBase(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	checkf(IsValid(NewCauser), TEXT("Failed to get skill causer"));

	ASkillBase* skillBase = GetSkillFromSkillBaseMap(NewCauser, OwnerSkillInfo);
	if (IsValid(skillBase))
	{
		UpdateSkillInfo(skillBase, OwnerSkillInfo);
		return skillBase;
	}
	else
	{
		skillBase = MakeSkillBase(NewCauser, OwnerSkillInfo);
		UpdateSkillInfo(skillBase, OwnerSkillInfo);
		return skillBase;
	}
}

ASkillBase* USkillManagerBase::UpdateSkillInfo(ASkillBase* SkillBase, FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	if (SkillUpgradeInfoTable == nullptr) return nullptr;

	//SkillBase의 레벨 변화 여부 확인
	if (SkillBase->SkillInfo.SkillLevelStruct.SkillLevel == OwnerSkillInfo->SkillLevel) return SkillBase;
	SkillBase->SkillInfo.SkillLevelStruct.SkillLevel = OwnerSkillInfo->SkillLevel;

	//스킬 레벨 변경시 변화요소 확인을 위해 SkillUpgradeInfo 정보를 받아옴
	checkf(IsValid(SkillUpgradeInfoTable), TEXT("Failed to get FSkillUpgradeInfoStruct"));
	FString rowName = FString::FromInt(OwnerSkillInfo->SkillID);
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	uint8 currSkillLevel = OwnerSkillInfo->SkillLevel;

	//스킬 쿨타임 변화 여부 확인 / 반영
	SkillBase->SkillInfo.SkillLevelStruct.CoolTime = skillUpgradeInfo->CoolTimeByLevel[currSkillLevel];

	//스킬 강화 변수 변화 여부 확인 / 반영
	TArray<FName> keyList;
	skillUpgradeInfo->SkillVariableMap.GetKeys(keyList);
	if (keyList.Num() == 0) return SkillBase;
	for (FName key : keyList)
	{
		SkillBase->SkillInfo.SkillLevelStruct.SkillVariableMap.Add(key, skillUpgradeInfo->SkillVariableMap[key].VariableByLevel[currSkillLevel]);
	}
	return SkillBase;
}

ASkillBase* USkillManagerBase::GetSkillFromSkillBaseMap(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	if (SkillBaseMap.IsEmpty()) return nullptr;
	FSkillBaseListContainer* targetListInfo = SkillBaseMap.Find(NewCauser);
	if (targetListInfo == nullptr) return nullptr;
	if (targetListInfo->SkillBaseList.IsEmpty()) return nullptr;
	for (ASkillBase* skillBase : targetListInfo->SkillBaseList)
	{
		if (!(skillBase->SkillInfo.SkillID == OwnerSkillInfo->SkillID)) continue;
		else return skillBase;
	}
	return nullptr;
}

ASkillBase* USkillManagerBase::MakeSkillBase(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	checkf(IsValid(SkillInfoTable), TEXT("Failed to get SkillInfoDataTable"));
	ASkillBase* skillBase;

	FSkillInfoStruct* skillInfo = GetSkillInfoStructByOwnerSkillInfo(OwnerSkillInfo);
	skillInfo->Causer = NewCauser;

	if (skillInfo == nullptr) { UE_LOG(LogTemp, Log, TEXT("Failed to get SkillInfoStruct")); return nullptr; }

	checkf(IsValid(OwnerSkillInfo->SkillBaseClassRef), TEXT("Failed to get SkillBase class"));
	skillBase = Cast<ASkillBase>(GetWorld()->SpawnActor(OwnerSkillInfo->SkillBaseClassRef));
	skillBase->InitSkill(*skillInfo);
	if (SkillBaseMap.Contains(NewCauser))
	{
		SkillBaseMap.Find(NewCauser)->SkillBaseList.Add(skillBase);
	}
	else
	{
		FSkillBaseListContainer skillListContainer;
		skillListContainer.SkillBaseList.Add(skillBase);
		SkillBaseMap.Add(NewCauser, skillListContainer);
	}
	return skillBase;
}

void USkillManagerBase::RemoveSkillInSkillBaseMap(ACharacterBase* NewCauser)
{
	if (SkillBaseMap.IsEmpty()) return;
	if (SkillBaseMap.Contains(NewCauser))
	{
		SkillBaseMap.Remove(NewCauser);
	}
}

bool USkillManagerBase::IsSkillInfoStructValid(const FSkillInfoStruct& SkillInfo)
{
	if (SkillInfo.SkillID != 0) return true;
	return false;
}

FSkillInfoStruct USkillManagerBase::GetCurrSkillInfoByType(ESkillType SkillType)
{
	if (SkillUpgradeInfoTable == nullptr) return FSkillInfoStruct();
	AMainCharacterBase* mainCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!IsValid(mainCharacterRef)) return FSkillInfoStruct();
	if (!mainCharacterRef->WeaponComponent->GetWeaponState().SkillInfoMap.Contains(SkillType)) return FSkillInfoStruct();
	FOwnerSkillInfoStruct* ownerSkillInfo = &mainCharacterRef->WeaponComponent->WeaponState.SkillInfoMap[SkillType];
	if (ownerSkillInfo == nullptr) return FSkillInfoStruct();
	if (ownerSkillInfo->SkillID == 0) return FSkillInfoStruct();

	//기존에 만들어져 있는 스킬 베이스 중 원하는 스킬이 있는지 확인
	if (SkillBaseMap.Contains(mainCharacterRef))
	{
		for (ASkillBase* skillBase : SkillBaseMap.Find(mainCharacterRef)->SkillBaseList)
		{
			if (skillBase->SkillInfo.SkillID == ownerSkillInfo->SkillID)
			{
				return UpdateSkillInfo(skillBase, ownerSkillInfo)->SkillInfo;
			}
		}
	}

	//기존에 만들어져 있는 스킬 베이스가 없다면 구조체를 생성
	//ID에 따른 스킬 기본 구조 불러오기

	FString rowName = FString::FromInt(ownerSkillInfo->SkillID);
	FSkillInfoStruct* skillInfo = SkillInfoTable->FindRow<FSkillInfoStruct>(FName(rowName), rowName);
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	
	//스킬 레벨 반영
	skillInfo->SkillLevelStruct.SkillLevel = ownerSkillInfo->SkillLevel;

	//스킬 쿨타임 변화 여부 확인 / 반영
	skillInfo->SkillLevelStruct.CoolTime = skillUpgradeInfo->CoolTimeByLevel[ownerSkillInfo->SkillLevel];

	//스킬 강화 변수 변화 여부 확인 / 반영
	TArray<FName> keyList;
	skillUpgradeInfo->SkillVariableMap.GetKeys(keyList);
	if (keyList.Num() == 0) return *skillInfo;
	for (FName key : keyList)
	{
		skillInfo->SkillLevelStruct.SkillVariableMap.Add(key, skillUpgradeInfo->SkillVariableMap[key].VariableByLevel[ownerSkillInfo->SkillLevel]);
	}

	return *skillInfo;			
}

int32 USkillManagerBase::GetCurrSkillIDByType(ESkillType SkillType)
{
	FSkillInfoStruct skillInfo = GetCurrSkillInfoByType(SkillType);
	if (!IsSkillInfoStructValid(skillInfo)) return 0;
	return skillInfo.SkillID;
}

UTexture2D* USkillManagerBase::GetCurrSkillImageByType(ESkillType SkillType)
{
	FSkillInfoStruct skillInfo= GetCurrSkillInfoByType(SkillType);
	if(!IsSkillInfoStructValid(skillInfo)) return nullptr;
	return skillInfo.SkillAssetStruct.IconImage;
}

uint8 USkillManagerBase::GetCurrSkillLevelByType(ESkillType SkillType)
{
	FSkillInfoStruct skillInfo = GetCurrSkillInfoByType(SkillType);
	if (!IsSkillInfoStructValid(skillInfo)) return 0;
	return skillInfo.SkillLevelStruct.SkillLevel;
}

uint8 USkillManagerBase::GetCurrSkillLevelByID(int32 SkillID)
{
	ESkillType skillType = GetSkillInfoStructBySkillID(SkillID).SkillType;
	AMainCharacterBase* mainCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	TMap<ESkillType, FOwnerSkillInfoStruct> skillInfoMap = mainCharacterRef->WeaponComponent->GetWeaponState().SkillInfoMap;
	if(!skillInfoMap.Contains(skillType)) return 0;
	if(skillInfoMap.Find(skillType)->SkillID != SkillID) return 0;
	return GetCurrSkillLevelByType(skillType);
}

float USkillManagerBase::GetCurrSkillCoolTimeByType(ESkillType SkillType)
{
	FSkillInfoStruct skillInfo = GetCurrSkillInfoByType(SkillType);
	if (!IsSkillInfoStructValid(skillInfo)) return 0;
	return skillInfo.SkillLevelStruct.CoolTime;
}

FSkillInfoStruct* USkillManagerBase::GetSkillInfoStructByOwnerSkillInfo(FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	if (!IsValid(SkillInfoTable)) { UE_LOG(LogTemp, Log, TEXT("There's no SkillInfoTable")); return nullptr; }

	FString rowName = FString::FromInt(OwnerSkillInfo->SkillID);
	FSkillInfoStruct* skillInfo = SkillInfoTable->FindRow<FSkillInfoStruct>(FName(rowName), rowName);
	return skillInfo;
}

FSkillInfoStruct USkillManagerBase::GetSkillInfoStructBySkillID(int32 SkillID)
{
	if (!IsValid(SkillInfoTable)) { UE_LOG(LogTemp, Log, TEXT("There's no SkillInfoTable")); return FSkillInfoStruct(); }

	FString rowName = FString::FromInt(SkillID);
	FSkillInfoStruct* skillInfo = SkillInfoTable->FindRow<FSkillInfoStruct>(FName(rowName), rowName);
	if (IsSkillInfoStructValid(*skillInfo))
	{
		return *skillInfo;
	}
	return FSkillInfoStruct();
}

FSkillUpgradeInfoStruct USkillManagerBase::GetSkillUpgradeInfoStructBySkillID(int32 SkillID)
{
	if (!IsValid(SkillUpgradeInfoTable)) 
	{ 
		UE_LOG(LogTemp, Log, TEXT("There's no SkillUpgradeInfoTable")); 
		return FSkillUpgradeInfoStruct(); 
	}

	FString rowName = FString::FromInt(SkillID);
	FSkillUpgradeInfoStruct* skillUpgradeInfo = SkillUpgradeInfoTable->FindRow<FSkillUpgradeInfoStruct>(FName(rowName), rowName);
	if (skillUpgradeInfo != nullptr)
	{
		return *skillUpgradeInfo;
	}
	return FSkillUpgradeInfoStruct();
}

void USkillManagerBase::SetSkillBlockState(ACharacterBase* Owner, ESkillType SkillType, bool bNewBlockState)
{
	if (!IsValid(Owner) || !IsValid(Owner->WeaponComponent)) return;

	FOwnerSkillInfoStruct* ownerSkillInfo = Owner->WeaponComponent->WeaponState.SkillInfoMap.Find(SkillType);
	if (ownerSkillInfo)
	{
		ownerSkillInfo->bSkillBlocked = bNewBlockState;
	}
}