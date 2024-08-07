// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySkillManagerComponent.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SkillSystem/SkillBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"	
#include "../../Character/Component/WeaponComponentBase.h"

UEnemySkillManagerComponent::UEnemySkillManagerComponent()
{
}

void UEnemySkillManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEnemySkillManagerComponent::InitSkillManager()
{
	Super::InitSkillManager();
	ClearAllSkill();
}

void UEnemySkillManagerComponent::InitSkillMap()
{
	Super::InitSkillMap();
}

bool UEnemySkillManagerComponent::TrySkill(int32 SkillID)
{
	return Super::TrySkill(SkillID);
}

bool UEnemySkillManagerComponent::AddSkill(int32 SkillID)
{
	UE_LOG(LogTemp, Warning, TEXT("AddSkill #1"));
	if (!Super::AddSkill(SkillID)) return false;

	UE_LOG(LogTemp, Warning, TEXT("AddSkill #2"));
	FString rowName = FString::FromInt(SkillID);
	FSkillStatStruct* skillStat = SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
	checkf(skillStat != nullptr, TEXT("SkillStat is not valid"));

	UE_LOG(LogTemp, Warning, TEXT("AddSkill #3"));

	ASkillBase* skillBase = Cast<ASkillBase>(GetWorld()->SpawnActor(skillStat->SkillBaseClassRef));
	if (!IsValid(skillBase)) return false;
	skillBase->InitSkill(*skillStat, this);

	UE_LOG(LogTemp, Warning, TEXT("AddSkill #4"));

	//Enemy 전용 스킬이 아니라면 Return
	if (skillBase->SkillStat.SkillWeaponStruct.SkillType != ESkillType::E_Enemy) return false;
	ESkillType skillType = skillBase->SkillStat.SkillWeaponStruct.SkillType;

	UE_LOG(LogTemp, Warning, TEXT("AddSkill #5"));

	//OwnedSkillIDList에 추가
	OwnedSkillInfoMap.Add(SkillID, skillBase);

	return true;
}

bool UEnemySkillManagerComponent::RemoveSkill(int32 SkillID)
{
	if (!Super::RemoveSkill(SkillID)) return false;
	if (!OwnedSkillInfoMap.Contains(SkillID)) return false;

	//OwnedSkillIDList에서 제거
	OwnedSkillInfoMap.Remove(SkillID);
	return true;
}

bool UEnemySkillManagerComponent::UpgradeSkill(int32 SkillID, uint8 NewLevel)
{
	return Super::UpgradeSkill(SkillID, NewLevel);
}

void UEnemySkillManagerComponent::ClearAllSkill()
{
	OwnedSkillInfoMap.Empty();
}

bool UEnemySkillManagerComponent::IsSkillValid(int32 SkillID)
{
	UE_LOG(LogTemp, Warning, TEXT("IsSkillValid__ #1 %d"), SkillID);
	if (!OwnedSkillInfoMap.Contains(SkillID)) return false;
	UE_LOG(LogTemp, Warning, TEXT("IsSkillValid__ #2"));
	ASkillBase* skillBase = *OwnedSkillInfoMap.Find(SkillID);
	return IsValid(skillBase);	
}

ASkillBase* UEnemySkillManagerComponent::GetOwnSkillBase(int32 SkillID)
{
	if (!OwnedSkillInfoMap.Contains(SkillID)) return false;
	ASkillBase* skillBase = *OwnedSkillInfoMap.Find(SkillID);
	return skillBase;
}
