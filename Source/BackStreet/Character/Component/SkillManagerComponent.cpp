// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerComponent.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SkillSystem/SkillBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"	
#include "../../Character/Component/WeaponComponentBase.h"

// Sets default values for this component's properties
USkillManagerComponent::USkillManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void USkillManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	InitSkillManager();
}

void USkillManagerComponent::InitSkillManager()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	SkillStatTable = GameModeRef.Get()->SkillStatTable;
	checkf(IsValid(SkillStatTable), TEXT("Failed to get SkillStatDataTable"));
	OwnerCharacterRef->WeaponComponent->OnWeaponUpdated.AddDynamic(this, &USkillManagerComponent::InitSkillMap);
}

void USkillManagerComponent::InitSkillMap()
{
	SkillInventoryMap.Reset();
	EquipedSkillMap.Reset();

	for (ESkillType skillType : OwnerCharacterRef->WeaponComponent->GetWeaponStat().SkillTypeList)
	{
		SkillInventoryMap.Add(skillType, FSkillListContainer());
		EquipedSkillMap.Add(skillType, 0);
	}
	return;
}

bool USkillManagerComponent::TrySkill(int32 SkillID)
{
	if (!IsSkillValid(SkillID)) return false;
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if (!IsValid(skillBase))
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponent::TrySkill(%d) : Skill Base Not Found"), SkillID);
		return false;
	}
	if (skillBase->SkillState.bIsBlocked) return false;
	if (skillBase->SkillStat.SkillLevelStatStruct.bIsLevelValid)
	{
		if(skillBase->SkillState.SkillLevelStateStruct.SkillLevel == 0) return false;
	}
	OwnerCharacterRef->SetActionState(ECharacterActionType::E_Skill);
	skillBase->ActivateSkill();
	OnSkillActivated.Broadcast(SkillID);
	return true;
}

bool USkillManagerComponent::AddSkill(int32 SkillID)
{
	if (IsSkillValid(SkillID)) return false;

	UE_LOG(LogTemp, Warning, TEXT("Add Skill #1 $d"), SkillID);

	FString rowName = FString::FromInt(SkillID);
	FSkillStatStruct* skillStat = SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
	checkf(skillStat != nullptr, TEXT("SkillStat is not valid"));

	UE_LOG(LogTemp, Warning, TEXT("Add Skill #2"));

	ASkillBase* skillBase = Cast<ASkillBase>(GetWorld()->SpawnActor(skillStat->SkillBaseClassRef));
	if(!IsValid(skillBase)) return false;
	skillBase->InitSkill(*skillStat, this);
	if (skillBase->SkillStat.SkillWeaponStruct.SkillType == ESkillType::E_None) return false;

	UE_LOG(LogTemp, Warning, TEXT("Add Skill #3"));

	ESkillType skillType = skillBase->SkillStat.SkillWeaponStruct.SkillType;

	UE_LOG(LogTemp, Warning, TEXT("Add Skill #4"));

	//SkillInventoryMap에 추가
	if(!SkillInventoryMap.Contains(skillType)) false;
	FSkillListContainer* skillListContainer = SkillInventoryMap.Find(skillType); 
	if (skillListContainer)
	{
		skillListContainer->SkillBaseList.AddUnique(skillBase);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Add Skill #5"));

	//EquipedSkillMap에 장착되어 있는 스킬이 없다면 추가
	UE_LOG(LogTemp, Log, TEXT("skilltype : %d"), skillType);
	UE_LOG(LogTemp, Log, TEXT("skilltype : %d"), EquipedSkillMap.Find(skillType));
	if (EquipedSkillMap.Find(skillType) == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("skillID : %d"), EquipedSkillMap.Find(skillType));
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

bool USkillManagerComponent::RemoveSkill(int32 SkillID)
{
	if (!IsSkillValid(SkillID)) return true;
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if (!IsValid(skillBase))
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponent::RemoveSkill(%d) : Skill Base Not Found"), SkillID);
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

bool USkillManagerComponent::UpgradeSkill(int32 SkillID, uint8 NewLevel)
{
	if (!IsSkillValid(SkillID)) return false;
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if(!IsValid(skillBase)) return false;
	skillBase->SkillState.SkillLevelStateStruct.SkillLevel = NewLevel;
	skillBase->SkillState.SkillLevelStateStruct.SkillVariableMap = skillBase->SkillStat.SkillLevelStatStruct.LevelInfo[NewLevel].SkillVariableMap;
	//쿨타임이 레벨에 따라 달라지는 경우 수정
	if (skillBase->SkillState.SkillLevelStateStruct.SkillVariableMap.Contains("CoolTime"))
	{
		skillBase->SkillStat.SkillLevelStatStruct.CoolTime = 
		*skillBase->SkillStat.SkillLevelStatStruct.LevelInfo[NewLevel].SkillVariableMap.Find("CoolTime");
	}
	OnSkillUpdated.Broadcast();
	return true;
}

void USkillManagerComponent::UpdateObtainableSkillMap()
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

bool USkillManagerComponent::EquipSkill(int32 NewSkillID)
{
	if(!IsSkillValid(NewSkillID)) return false;
	else
	{
		EquipedSkillMap.Add(GetSkillInfo(NewSkillID).SkillWeaponStruct.SkillType, NewSkillID);
		return true;
	}
}

FSkillStatStruct USkillManagerComponent::GetSkillInfo(int32 SkillID)
{
	FString rowName = FString::FromInt(SkillID);
	//checkf(IsValid(SkillStatTable), TEXT("SkillStatTable is not valid"));
	if (SkillStatTable)
	{
		FSkillStatStruct* skillStat = SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
		checkf(skillStat != nullptr, TEXT("SkillStat is not valid"));
		return *skillStat;
	}
	return FSkillStatStruct();
}

ESkillType USkillManagerComponent::GetSkillTypeInfo(int32 SkillID)
{
	return GetSkillInfo(SkillID).SkillWeaponStruct.SkillType;
}

ASkillBase* USkillManagerComponent::GetOwnSkillBase(int32 SkillID)
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

bool USkillManagerComponent::IsSkillValid(int32 SkillID)
{
	if(GetOwnSkillBase(SkillID)==nullptr) return false;
	return true;
}

FSkillStatStruct USkillManagerComponent::GetOwnSkillStat(int32 SkillID)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillStat;
}

FSkillStateStruct USkillManagerComponent::GetOwnSkillState(int32 SkillID)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillState;
}

TArray<uint8> USkillManagerComponent::GetRequiredMatAmount(int32 SkillID, uint8 NewSkillLevel)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillStat.SkillLevelStatStruct.LevelInfo[NewSkillLevel].RequiredMaterial;
}
