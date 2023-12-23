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
	static ConstructorHelpers::FObjectFinder<UDataTable> statInfoTableFinder(TEXT("/Game/Skill/Data/D_SkillInfo.D_SkillInfo"));
	checkf(statInfoTableFinder.Succeeded(), TEXT("skillInfoTable 탐색에 실패했습니다."));
	SkillInfoTable = statInfoTableFinder.Object;
}

void USkillManagerBase::InitSkillManagerBase(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
}

TArray<ASkillBase*> USkillManagerBase::ActivateSkill(AActor* NewCauser, ACharacterBase* NewTarget)
{
	SkillList.Empty();
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	if (!IsValid(causer)) return SkillList;
	
	if (causer->ActorHasTag("Player"))
	{
		SetMainCharacterSkillGrade(causer);
	}
	else if (causer->ActorHasTag("Enemy"))
	{
		SetEnemyCharacterSkillGrade(causer);
	}
	else return SkillList;
	
	if (!IsValidGrade(causer)) return SkillList;
	FSkillSetInfo skillSetInfo =  causer->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo;
	for (uint8 idx = 0; idx < skillSetInfo.SkillIDList.Num(); idx++)
	{
		if (idx != 0)
		{
			DelaySkillInterval(idx);
		}
		ASkillBase* skill = ComposeSkillMap(causer, skillSetInfo.SkillIDList[idx]);
		skill->InitSkill(NewCauser, NewTarget);
		SkillList.Add(skill);
	}
	return SkillList;
}

void USkillManagerBase::DestroySkill(AActor* NewCauser, TArray<ASkillBase*> UsedSkillList)
{
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	if (!IsValid(causer) || !IsValid(causer->GetCurrentWeaponRef())) return;
	for (ASkillBase* skill : UsedSkillList) {
		if (IsValid(skill))
		{
			FWeaponStatStruct currWeaponStat = causer->GetCurrentWeaponRef()->GetWeaponStat();
			currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_None;
			causer->GetCurrentWeaponRef()->SetWeaponStat(currWeaponStat);
			skill->DestroySkill();
		}
	}
}

void USkillManagerBase::SetMainCharacterSkillGrade(AActor* NewCauser) 
{
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	if (!IsValid(causer)||!IsValid(causer->GetCurrentWeaponRef())) return;
	
	float currSkillGauge = Cast<AMainCharacterBase>(causer)->GetCharacterState().CharacterCurrSkillGauge;
	FWeaponStatStruct currWeaponStat = causer->GetCurrentWeaponRef()->GetWeaponStat();
	FCharacterStateStruct currCharacterState = causer->GetCharacterState();

	//Grade확인
	if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillCommonReq, currWeaponStat.SkillGaugeInfo.SkillRareReq, true, false))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Common;
		currCharacterState.CharacterCurrSkillGauge -= currWeaponStat.SkillGaugeInfo.SkillCommonReq;
		causer->UpdateCharacterState(currCharacterState);
		UE_LOG(LogTemp, Log, TEXT("CommonSkill"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillRareReq, currWeaponStat.SkillGaugeInfo.SkillEpicReq, true, false))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Rare;
		currCharacterState.CharacterCurrSkillGauge -= currWeaponStat.SkillGaugeInfo.SkillRareReq;
		causer->UpdateCharacterState(currCharacterState);
		UE_LOG(LogTemp, Log, TEXT("RareSkill"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillEpicReq, currWeaponStat.SkillGaugeInfo.SkillRegendReq, true, false))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Epic;
		currCharacterState.CharacterCurrSkillGauge -= currWeaponStat.SkillGaugeInfo.SkillEpicReq;
		causer->UpdateCharacterState(currCharacterState);
		UE_LOG(LogTemp, Log, TEXT("EpicSkill"));
	}
	else if (currSkillGauge >= currWeaponStat.SkillGaugeInfo.SkillRegendReq)
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Regend;
		currCharacterState.CharacterCurrSkillGauge -= currWeaponStat.SkillGaugeInfo.SkillRegendReq;
		causer->UpdateCharacterState(currCharacterState);
		UE_LOG(LogTemp, Log, TEXT("RegendSkill"));
	}
	else
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_None;
	}
	causer->GetCurrentWeaponRef()->SetWeaponStat(currWeaponStat);
}

void USkillManagerBase::SetEnemyCharacterSkillGrade(AActor* NewCauser)
{
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	if (!IsValid(causer) || !IsValid(causer->GetCurrentWeaponRef())) return;
	
	FWeaponStatStruct currWeaponStat = causer->GetCurrentWeaponRef()->GetWeaponStat();

	if (causer->ActorHasTag("Easy"))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Common;
	}
	else if (causer->ActorHasTag("Nomal"))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Rare;
	}
	else if (causer->ActorHasTag("Hard"))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Epic;
	}
	else if (causer->ActorHasTag("Extreme"))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Regend;
	}
	else
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_None;
	}
	causer->GetCurrentWeaponRef()->SetWeaponStat(currWeaponStat);
}


bool USkillManagerBase::IsValidGrade(AActor* NewCauser)
{
	if(Cast<ACharacterBase>(NewCauser)->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillGrade ==ESkillGrade::E_None) return false;
	else return true;
}

void USkillManagerBase::DelaySkillInterval(uint8 NewIndex)
{

}

ASkillBase* USkillManagerBase::ComposeSkillMap(AActor* NewCauser, int32 NewSkillID)
{
	ASkillBase* skill =nullptr;
	if(Cast<AMainCharacterBase>(NewCauser)->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillGrade == ESkillGrade::E_None)
		return skill;

	if (SkillRefMap.Contains(NewSkillID))
	{
		skill = *SkillRefMap.Find(NewSkillID);
	}
	else
	{
		skill = MakeSkillBase(NewSkillID);
		SkillRefMap.Add(NewSkillID, skill);
	}
	return skill;
}

ASkillBase* USkillManagerBase::MakeSkillBase(int32 NewSkillID)
{
	FString skillkey = UKismetStringLibrary::Conv_IntToString(NewSkillID);
	FSkillInfoStruct* skillInfo = SkillInfoTable->FindRow<FSkillInfoStruct>((FName)skillkey, skillkey);
	if (skillInfo != nullptr)
	{
		ASkillBase* skillBaseRef = Cast<ASkillBase>(GetWorld()->SpawnActor(skillInfo->SkillBaseClassRef));
		if (IsValid(skillBaseRef))
		{
			skillBaseRef->SetSkillManagerRef(this);
			SkillRefMap.Add(NewSkillID, skillBaseRef);
			return skillBaseRef;
		}
	}
	return nullptr;
}

void USkillManagerBase::ClearAllTimerHandle()
{
	GamemodeRef.Get()->GetWorldTimerManager().ClearTimer(SkillTimerHandle);
}


