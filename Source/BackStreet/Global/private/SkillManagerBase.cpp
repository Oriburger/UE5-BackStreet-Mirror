// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/SkillManagerBase.h"
#include "../public/BackStreetGameModeBase.h"
#include "../../SkillSystem/public/SkillBase.h"
#include "../../Character/public/CharacterBase.h"
#include "Kismet/KismetStringLibrary.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"

USkillManagerBase::USkillManagerBase()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> skillInfoTableFinder(TEXT("/Game/System/SkillManager/Data/D_SkillInfo.D_SkillInfo"));
	checkf(skillInfoTableFinder.Succeeded(), TEXT("SkillInfoTable class discovery failed."));
	SkillInfoTable = skillInfoTableFinder.Object;

}

void USkillManagerBase::InitSkillManagerBase(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
}

void USkillManagerBase::TrySkill(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	ASkillBase* skillBase = SpawnSkillBase(NewCauser, OwnerSkillInfo);
	if(!IsValid(skillBase)) return;
	if (skillBase->SkillInfo.SkillGradeStruct.bIsGradeValid)
	{
		if (!skillBase->CheckSkillGauge()) { UE_LOG(LogTemp, Log, TEXT("Need More SkillGauge")); return; }
	}
	NewCauser->SetActionState(ECharacterActionType::E_Skill);
	skillBase->ActivateSkill();
}

ASkillBase* USkillManagerBase::SpawnSkillBase(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	checkf(IsValid(NewCauser), TEXT("Failed to get skill causer"));

	ASkillBase* skillBase = GetSkillFromSkillBaseMap(NewCauser, OwnerSkillInfo);
	if (IsValid(skillBase)) return skillBase;
	else
	{
		checkf(IsValid(SkillInfoTable), TEXT("Failed to get SkillInfoDataTable"));
		
		FSkillInfoStruct* skillInfo = GetSkillInfoStructBySkillID(OwnerSkillInfo);
		skillInfo->Causer = NewCauser;

		if (skillInfo == nullptr) { UE_LOG(LogTemp, Log, TEXT("Failed to get SkillInfoStruct")); return nullptr; }

		checkf(IsValid(OwnerSkillInfo->SkillBaseClassRef), TEXT("Failed to get SkillBase class"));
		skillBase = Cast<ASkillBase>(GetWorld()->SpawnActor(OwnerSkillInfo->SkillBaseClassRef));
		skillBase->InitSkill(*skillInfo);
		return skillBase;
	}
}

ASkillBase* USkillManagerBase::GetSkillFromSkillBaseMap(ACharacterBase* NewCauser, FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	if(SkillBaseMap.IsEmpty()) return nullptr;
	if(SkillBaseMap.Find(NewCauser)->SkillBaseList.IsEmpty()) return nullptr;
	for (ASkillBase* skillBase : SkillBaseMap.Find(NewCauser)->SkillBaseList)
	{
		if(!(skillBase->SkillInfo.SkillID == OwnerSkillInfo->SkillID)) continue;
		else return skillBase;
	}
	return nullptr;
}

void USkillManagerBase::RemoveSkillInSkillBaseMap(ACharacterBase* NewCauser)
{
	if (SkillBaseMap.IsEmpty()) return;
	if (SkillBaseMap.Contains(NewCauser))
	{
		SkillBaseMap.Remove(NewCauser);
	}
}

FSkillInfoStruct* USkillManagerBase::GetSkillInfoStructBySkillID(FOwnerSkillInfoStruct* OwnerSkillInfo)
{
	if (!IsValid(SkillInfoTable)) { UE_LOG(LogTemp, Log, TEXT("There's no SkillInfoTable")); return nullptr; }

	FString rowName = FString::FromInt(OwnerSkillInfo->SkillID);
	return SkillInfoTable->FindRow<FSkillInfoStruct>(FName(rowName), rowName);
}


