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
	checkf(statInfoTableFinder.Succeeded(), TEXT("skillInfoTable Ž���� �����߽��ϴ�."));
	SkillInfoTable = statInfoTableFinder.Object;
}

void USkillManagerBase::InitSkillManagerBase(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
}

void USkillManagerBase::ActivateSkill(AActor* NewCauser, TArray<ACharacterBase*> NewTargetList)
{
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	if (!IsValid(causer)) return;
	
	if (causer->ActorHasTag("Player"))
	{
		SetMainCharacterSkillGrade(causer);
	}
	else if (causer->ActorHasTag("Enemy"))
	{
		SetEnemyCharacterSkillGrade(causer);
	}
	else return;
	
	if (!IsValidGrade(causer)) return;
	FSkillSetInfo skillSetInfo =  causer->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo;
	for (uint8 idx = 0; idx < skillSetInfo.SkillIDList.Num(); idx++)
	{
		ASkillBase* skill = ComposeSkillMap(causer, skillSetInfo.SkillIDList[idx]);
		int32 skillID = causer->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillIDList[idx];
		float skillStartTiming = skillSetInfo.TotalSkillPlayTime * skillSetInfo.SkillStartTimingRateList[idx];
		skill->InitSkill(NewCauser, NewTargetList, skillID, skillStartTiming);

	}
	return;
}

void USkillManagerBase::SetMainCharacterSkillGrade(AActor* NewCauser) 
{
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	if (!IsValid(causer)||!IsValid(causer->GetCurrentWeaponRef())) return;
	
	float currSkillGauge = Cast<AMainCharacterBase>(causer)->GetCharacterState().CharacterCurrSkillGauge;
	FWeaponStatStruct currWeaponStat = causer->GetCurrentWeaponRef()->GetWeaponStat();
	FCharacterStateStruct currCharacterState = causer->GetCharacterState();

	//GradeȮ��
	if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillCommonReq, currWeaponStat.SkillGaugeInfo.SkillRareReq, true, false))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Common;
		currCharacterState.CharacterCurrSkillGauge -= currWeaponStat.SkillGaugeInfo.SkillCommonReq;
		causer->UpdateCharacterState(currCharacterState);
		UE_LOG(LogTemp, Log, TEXT("CommonSkill"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillRareReq, currWeaponStat.SkillGaugeInfo.SkillLegendReq, true, false))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Rare;
		currCharacterState.CharacterCurrSkillGauge -= currWeaponStat.SkillGaugeInfo.SkillRareReq;
		causer->UpdateCharacterState(currCharacterState);
		UE_LOG(LogTemp, Log, TEXT("RareSkill"));
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, currWeaponStat.SkillGaugeInfo.SkillLegendReq, currWeaponStat.SkillGaugeInfo.SkillMythicReq, true, false))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Legend;
		currCharacterState.CharacterCurrSkillGauge -= currWeaponStat.SkillGaugeInfo.SkillLegendReq;
		causer->UpdateCharacterState(currCharacterState);
		UE_LOG(LogTemp, Log, TEXT("LegendSkill"));
	}
	else if (currSkillGauge >= currWeaponStat.SkillGaugeInfo.SkillMythicReq)
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Mythic;
		currCharacterState.CharacterCurrSkillGauge -= currWeaponStat.SkillGaugeInfo.SkillMythicReq;
		causer->UpdateCharacterState(currCharacterState);
		UE_LOG(LogTemp, Log, TEXT("MythicSkill"));
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
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Legend;
	}
	else if (causer->ActorHasTag("Extreme"))
	{
		currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_Mythic;
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

ASkillBase* USkillManagerBase::ComposeSkillMap(AActor* NewCauser, int32 NewSkillID)
{
	ASkillBase* skill =nullptr;
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	FSkillSetInfo skillSetInfo = causer->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo;
	TMap<int32, class ASkillBase* > skillRefMap = skillSetInfo.SkillRefMap;
	if (causer->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillGrade == ESkillGrade::E_None)
	{
		return skill;
	}
	if (causer->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillRefMap.Contains(NewSkillID))
	{
		skill = *skillRefMap.Find(NewSkillID);
	}
	else
	{
		skill = MakeSkillBase(NewCauser, NewSkillID);
	}
	return skill;
}

ASkillBase* USkillManagerBase::MakeSkillBase(AActor* NewCauser, int32 NewSkillID)
{
	ACharacterBase* causer = Cast<ACharacterBase>(NewCauser);
	FString skillkey = UKismetStringLibrary::Conv_IntToString(NewSkillID);
	FSkillInfoStruct* skillInfo = SkillInfoTable->FindRow<FSkillInfoStruct>((FName)skillkey, skillkey);
	if (skillInfo != nullptr)
	{
		ASkillBase* skillBaseRef = Cast<ASkillBase>(GetWorld()->SpawnActor(skillInfo->SkillBaseClassRef));
		if (IsValid(skillBaseRef))
		{
			FWeaponStatStruct weaponStat = causer->GetCurrentWeaponRef()->GetWeaponStat();
			FSkillSetInfo skillSetInfo =  weaponStat.SkillSetInfo;

			skillBaseRef->SetSkillManagerRef(this);
			skillSetInfo.SkillRefMap.Add(NewSkillID, skillBaseRef);
			weaponStat.SkillSetInfo = skillSetInfo;
			causer->GetCurrentWeaponRef()->SetWeaponStat(weaponStat);

			return skillBaseRef;
		}
	}
	return nullptr;
}
