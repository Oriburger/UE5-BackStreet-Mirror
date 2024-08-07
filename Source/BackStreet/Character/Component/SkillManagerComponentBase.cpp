// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerComponentBase.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SkillSystem/SkillBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"	
#include "../../Character/Component/WeaponComponentBase.h"

// Sets default values for this component's properties
USkillManagerComponentBase::USkillManagerComponentBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void USkillManagerComponentBase::BeginPlay()
{
	Super::BeginPlay();
	InitSkillManager();
}

void USkillManagerComponentBase::InitSkillManager()
{
	//Initialize the owner character ref
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner());
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	SkillStatTable = GameModeRef.Get()->SkillStatTable;
	checkf(IsValid(SkillStatTable), TEXT("Failed to get SkillStatDataTable"));
	OwnerCharacterRef->WeaponComponent->OnWeaponUpdated.AddDynamic(this, &USkillManagerComponentBase::InitSkillMap);
}

void USkillManagerComponentBase::InitSkillMap() {}

bool USkillManagerComponentBase::TrySkill(int32 SkillID)
{
	if (!IsSkillValid(SkillID)) return false;
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	if (!IsValid(skillBase))
	{
		UE_LOG(LogTemp, Error, TEXT("USkillManagerComponentBase::TrySkill(%d) : Skill Base Not Found"), SkillID);
		return false;
	}
	if (skillBase->SkillState.bIsBlocked) return false;
	if (skillBase->SkillStat.SkillLevelStatStruct.bIsLevelValid)
	{
		if (skillBase->SkillState.SkillLevelStateStruct.SkillLevel == 0) return false;
	}
	OwnerCharacterRef->SetActionState(ECharacterActionType::E_Skill);
	skillBase->ActivateSkill();
	OnSkillActivated.Broadcast(SkillID);
	return true;
}

bool USkillManagerComponentBase::AddSkill(int32 SkillID)
{
	if (IsSkillValid(SkillID)) return false;
	return true;
}

bool USkillManagerComponentBase::RemoveSkill(int32 SkillID)
{
	return true;
}

bool USkillManagerComponentBase::UpgradeSkill(int32 SkillID, uint8 NewLevel)
{
	UE_LOG(LogTemp, Warning, TEXT("UpgradeSkill #1 %d %d"), SkillID, NewLevel);
	if (!IsSkillValid(SkillID)) return false;
	UE_LOG(LogTemp, Warning, TEXT("UpgradeSkill #2"));
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	UE_LOG(LogTemp, Warning, TEXT("UpgradeSkill #3"));
	if (!IsValid(skillBase)) return false;
	UE_LOG(LogTemp, Warning, TEXT("UpgradeSkill #4"));
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

void USkillManagerComponentBase::ClearAllSkill() {}

FSkillStatStruct USkillManagerComponentBase::GetSkillInfo(int32 SkillID)
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

ESkillType USkillManagerComponentBase::GetSkillTypeInfo(int32 SkillID)
{
	return GetSkillInfo(SkillID).SkillWeaponStruct.SkillType;
}

ASkillBase* USkillManagerComponentBase::GetOwnSkillBase(int32 SkillID)
{
	return nullptr;
}

bool USkillManagerComponentBase::IsSkillValid(int32 SkillID)
{
	UE_LOG(LogTemp, Warning, TEXT("IsSkillValid #1"));
	if (GetOwnSkillBase(SkillID) == nullptr) return false;
	UE_LOG(LogTemp, Warning, TEXT("IsSkillValid #2"));
	return true;
}

FSkillStatStruct USkillManagerComponentBase::GetOwnSkillStat(int32 SkillID)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillStat;
}

FSkillStateStruct USkillManagerComponentBase::GetOwnSkillState(int32 SkillID)
{
	ASkillBase* skillBase = GetOwnSkillBase(SkillID);
	checkf(IsValid(skillBase), TEXT("Failed Find Skill"));
	return skillBase->SkillState;
}