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
	//추후 초기화를 위한 Temp 배열을 초기화
	/*for (int idx = 0; idx <= DEBUFF_DAMAGE_TIMER_IDX; idx++)
	{
		TempTimerHandleList.Add(FTimerHandle());
		TempResetValueList.Add(0.0f);
	}*/

	static ConstructorHelpers::FObjectFinder<UDataTable> statInfoTableFinder(TEXT("/Game/Skill/Data/D_SkillInfo.D_SkillInfo"));
	checkf(statInfoTableFinder.Succeeded(), TEXT("skillInfoTable 탐색에 실패했습니다."));
	SkillInfoTable = statInfoTableFinder.Object;
}

void USkillManagerBase::InitSkillManagerBase(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;

	/*for (int newTimerIdx = 0; newTimerIdx <= MAX_DEBUFF_IDX; newTimerIdx += 1)
	{
		TempTimerHandleList.Add(FTimerHandle());
		TempResetValueList.Add(0.0f);
	}*/
	GamemodeRef = NewGamemodeRef;
}

void USkillManagerBase::ActivateSkill(AActor* Causer, ACharacterBase* Target)
{
	if (!IsValid(Causer)) return;

	SetSkillSet(Causer);

	for (uint8 idx = 0; idx < SkillSetInfo.SkillIDList.Num(); idx++)
	{
		int32 skillID = SkillSetInfo.SkillIDList[idx];
		if (skillID == 0)
		{
			ComposeSkillMap(skillID, Causer, Target);
		}
		else
		{
			GetDelayInterval(idx, SkillSetInfo.SkillIntervalList);
			ComposeSkillMap(skillID, Causer, Target);
		}
	}
}

void USkillManagerBase::SetSkillSet(AActor* Causer) {
	ACharacterBase* causer = Cast<ACharacterBase>(Causer);
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

void USkillManagerBase::GetDelayInterval(int32 SkillListIdx, TArray<float> SkillIntervalList)
{
	float SkillInterval;

	if (SkillIntervalList.IsValidIndex(SkillListIdx) == true)
	{
		SkillInterval = SkillIntervalList[SkillListIdx];
		//delay 함수
	}
	else
	{

	}
}

void USkillManagerBase::ComposeSkillMap(int32 SkillID, AActor* Causer, ACharacterBase* Target)
{
	if(Cast<AMainCharacterBase>(Causer)->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillGrade == ESkillGrade::E_None)
		return;

	if (SkillRefMap.Contains(SkillID))
	{
		ASkillBase* skill = *SkillRefMap.Find(SkillID);
		if (IsValid(skill))
		{
			skill->InitSkill(Causer, Target, SkillID, SkillSetInfo.SkillGrade);
		}
	}
	else
	{
		ASkillBase* skill = MakeSkillBase(SkillID);
		if (IsValid(skill))
		{
			skill->InitSkill(Causer, Target, SkillID, SkillSetInfo.SkillGrade);
		}
	}
}

ASkillBase* USkillManagerBase::MakeSkillBase(int32 SkillID)
{
	FString skillkey = UKismetStringLibrary::Conv_IntToString(SkillID);
	FSkillInfoStruct* skillInfo = SkillInfoTable->FindRow<FSkillInfoStruct>((FName)skillkey, skillkey);
	if (skillInfo != nullptr)
	{
		ASkillBase* skillBaseRef = Cast<ASkillBase>(GetWorld()->SpawnActor(skillInfo->SkillBaseClassRef));
		if (IsValid(skillBaseRef))
		{
			skillBaseRef->SetSkillManagerRef(this);
			SkillRefMap.Add(SkillID, skillBaseRef);
			return skillBaseRef;
		}
	}
	return nullptr;
}


