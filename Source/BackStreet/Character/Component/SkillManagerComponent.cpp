// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillManagerComponent.h"
#include "../CharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SkillSystem/SkillBase.h"

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
}

bool USkillManagerComponent::TrySkill(int32 SkillID)
{
	ASkillBase* skillBase = GetSkillBaseByID(SkillID);
	if(skillBase == nullptr) return false;
	if (!skillBase->SkillState.bIsBlocked) return false;
	if (skillBase->SkillState.SkillLevelStruct.bIsLevelValid)
	{
		if(skillBase->SkillState.SkillLevelStruct.SkillLevel == 0) return false;
	}
	OwnerCharacterRef->SetActionState(ECharacterActionType::E_Skill);
	skillBase->ActivateSkill();

	return true;
}

bool USkillManagerComponent::AddSkill(int32 SkillID)
{
	if (SkillMap.Contains(SkillID)) return false;
	FString rowName = FString::FromInt(SkillID);
	FSkillStatStruct skillStat = *SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
	TWeakObjectPtr<ASkillBase> skillBase = Cast<ASkillBase>(GetWorld()->SpawnActor(skillStat.SkillBaseClassRef));
	SkillMap.Add(SkillID, skillBase);
	skillBase->InitSkill(skillStat, this);
	return true;
}

bool USkillManagerComponent::RemoveSkill(int32 SkillID)
{
	if (!SkillMap.Contains(SkillID)) return true;
	SkillMap.Remove(SkillID);
	return true;
}

TArray<int32> USkillManagerComponent::GetOwnSkillID()
{
	if(SkillMap.IsEmpty()) return TArray<int32>();
	TArray<int32> keys;
	SkillMap.GetKeys(keys);
	return keys;
}

ASkillBase* USkillManagerComponent::GetSkillBaseByID(int32 SkillID)
{
	if (!SkillMap.Contains(SkillID)) return nullptr;
	return SkillMap.Find(SkillID)->Get();
}




