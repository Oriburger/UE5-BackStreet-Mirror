// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySkillManagerComponent.h"

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
	return false;
}

bool UEnemySkillManagerComponent::RemoveSkill(int32 SkillID)
{
	return false;
}

bool UEnemySkillManagerComponent::UpgradeSkill(int32 SkillID, uint8 NewLevel)
{
	return Super::UpgradeSkill(SkillID, NewLevel);
}

void UEnemySkillManagerComponent::ClearAllSkill()
{
}

bool UEnemySkillManagerComponent::IsSkillValid(int32 SkillID)
{
	return Super::IsSkillValid(SkillID);
}
