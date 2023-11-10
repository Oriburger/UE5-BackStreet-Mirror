// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/SkillManagerBase.h"
#include "../public/BackStreetGameModeBase.h"
#include "../../SkillSystem/public/SkillBase.h"
#include "../../Character/public/CharacterBase.h"
#include "Kismet/KismetStringLibrary.h"

USkillManagerBase::USkillManagerBase()
{
	//추후 초기화를 위한 Temp 배열을 초기화
	/*for (int idx = 0; idx <= DEBUFF_DAMAGE_TIMER_IDX; idx++)
	{
		TempTimerHandleList.Add(FTimerHandle());
		TempResetValueList.Add(0.0f);
	}*/
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

void USkillManagerBase::ActivateSkill(FSkillSetStruct SkillSet, AActor* Causer, ACharacterBase* Target)
{
	if (!IsValid(Causer) || !IsValid(Target)) return;
	//ensure(IsValid(Causer)); 
	//check(IsValid(Causer));
	SkillSetStruct = SkillSet;
	SkillIDArray = SkillSetStruct.SkillIDList;
	SkillIntervalArray = SkillSetStruct.SkillIntervalList;

	for (uint8 idx = 0; idx < SkillIDArray.Num(); idx++)
	{
		int32 skillID = SkillIDArray[idx];

		if (skillID == 0)
		{
			ComposeSkillMap(skillID, Causer, Target);
		}
		else
		{
			GetDelayInterval(idx, SkillIntervalArray);
			ComposeSkillMap(skillID, Causer, Target);

		}
	}
	//임시코드
	//Target = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
}

float USkillManagerBase::GetDelayInterval(int32 SkillListIdx, TArray<float> SkillIntervalList)
{
	float SkillInterval;

	if (SkillIntervalList.IsValidIndex(SkillListIdx) == true)
	{
		SkillInterval = SkillIntervalList[SkillListIdx];
		//delay
		return SkillInterval;
	}
	else
	{
		return 0.0f;
	}
}

void USkillManagerBase::ComposeSkillMap(int32 SkillID, AActor* Causer, ACharacterBase* Target)
{
	if (SkillRefMap.Contains(SkillID) == true)
	{
		ASkillBase* skill = *SkillRefMap.Find(SkillID);
		if (IsValid(skill))
		{
			skill->InitSkill(Causer, Target, SkillSetStruct.SkillGrade);
		}
	}
	else
	{
		ASkillBase* skill = MakeSkillBase(SkillID);
		if (IsValid(skill))
		{
			skill->InitSkill(Causer, Target, SkillSetStruct.SkillGrade);
		}
	}
}

ASkillBase* USkillManagerBase::MakeSkillBase(int32 SkillID)
{
	UDataTable* D_SkillInfo = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/SkillSystem/Data/D_SkillInfo.D_SkillInfo'"));
	FString Skillkey = UKismetStringLibrary::Conv_IntToString(SkillID);
	FSkillInfoStruct* SkillInfo = D_SkillInfo->FindRow<FSkillInfoStruct>((FName)Skillkey, Skillkey);

	if (SkillInfo != nullptr)
	{
		ASkillBase* SkillBaseRef = Cast<ASkillBase>(GetWorld()->SpawnActor(SkillInfo->SkillBaseClassRef));
		SkillRefMap.Add(SkillID, SkillBaseRef);
		return SkillBaseRef;
	}
	return nullptr;
}

