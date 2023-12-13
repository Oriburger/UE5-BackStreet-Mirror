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
	if (!IsValid(Causer)) return SkillList;
	
	Causer = NewCauser;
	Target = NewTarget;

	SetSkillSet(Causer);

	for (uint8 idx = 0; idx < SkillSetInfo.SkillIDList.Num(); idx++)
	{
		if (idx != 0)
		{
			DelaySkillInterval(idx);
		}
		ASkillBase* skill = ComposeSkillMap(SkillSetInfo.SkillIDList[idx]);
		skill->InitSkill(Causer, Target, SkillSetInfo.SkillIDList[idx], SkillSetInfo.SkillGrade);
		SkillList.Add(skill);
	}
	return SkillList;
}

void USkillManagerBase::DestroySkill(TArray<ASkillBase*> UsedSkillList)
{
	for (ASkillBase* skill : UsedSkillList) {
		if (IsValid(skill))
		{
			skill->DestroySkill();
		}
	}
}

void USkillManagerBase::SetSkillSet(AActor* NewCauser) {
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	if (!IsValid(causer)||!IsValid(causer->GetCurrentWeaponRef())) return;

	//Player 인경우
	if (causer->ActorHasTag("Player")) {
		float currSkillGauge = Cast<AMainCharacterBase>(causer)->GetCharacterState().CharacterCurrSkillGauge;
		
		FWeaponStatStruct currWeaponStat = causer->GetCurrentWeaponRef()->GetWeaponStat();

		//Grade확인
		if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillCommonReq, currWeaponStat.SkillGaugeInfo.SkillRareReq, true, false))
		{
			currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Common;
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillRareReq, currWeaponStat.SkillGaugeInfo.SkillEpicReq, true, false))
		{
			currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Rare;
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillEpicReq, currWeaponStat.SkillGaugeInfo.SkillRegendReq, true, false))
		{
			currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Epic;
		}
		else if (currSkillGauge >= currWeaponStat.SkillGaugeInfo.SkillRegendReq)
		{
			currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Regend;
		}
		else
		{
			currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_None;
		}
		
		causer->GetCurrentWeaponRef()->SetWeaponStat(currWeaponStat);
		SkillSetInfo = currWeaponStat.SkillSetInfo;
	}

	//Enemy인 경우
	else if (causer->ActorHasTag("Enemy"))
	{
		if (SkillSetInfoMap.Contains(causer->GetCurrentWeaponRef()->GetWeaponStat().WeaponID))
		{
			SkillSetInfo = *SkillSetInfoMap.Find(causer->GetCurrentWeaponRef()->GetWeaponStat().WeaponID);
		}
		else
		{
			SkillSetInfo = causer->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo;

			if (causer->ActorHasTag("Easy"))
			{
				SkillSetInfo.SkillGrade = ESkillGrade::E_Common;
			}
			else if (causer->ActorHasTag("Nomal"))
			{
				SkillSetInfo.SkillGrade = ESkillGrade::E_Rare;
			}
			else if (causer->ActorHasTag("Hard"))
			{
				SkillSetInfo.SkillGrade = ESkillGrade::E_Epic;
			}
			else if (causer->ActorHasTag("Extreme"))
			{
				SkillSetInfo.SkillGrade = ESkillGrade::E_Regend;
			}
			else
			{
				SkillSetInfo.SkillGrade = ESkillGrade::E_None;
			}
			SkillSetInfoMap.Add(causer->GetCurrentWeaponRef()->GetWeaponStat().WeaponID, causer->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo);
		}
	}
}

void USkillManagerBase::DelaySkillInterval(uint8 NewIndex)
{
	 float SkillInterval = Cast<ACharacterBase> (Causer)->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillIntervalList[NewIndex];
	 GetWorld()->GetTimerManager().SetTimer(SkillTimerHandle, FTimerDelegate::CreateLambda([&]()
		 {
			 ClearAllTimerHandle();
		 }), SkillInterval, false);
}

ASkillBase* USkillManagerBase::ComposeSkillMap(int32 NewSkillID)
{
	ASkillBase* skill =nullptr;
	if(Cast<AMainCharacterBase>(Causer)->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillGrade == ESkillGrade::E_None)
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


