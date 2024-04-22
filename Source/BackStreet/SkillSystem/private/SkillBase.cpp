// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/SkillBase.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Global/public/SkillManagerBase.h"
#include "../../Global/public/AssetManagerBase.h"
#include "../../Character/public/CharacterBase.h"

// Sets default values
ASkillBase::ASkillBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	DefaultSceneRoot->SetupAttachment(RootComponent);
	SetRootComponent(DefaultSceneRoot);

}

// Called when the game starts or when spawned
void ASkillBase::BeginPlay()
{
	Super::BeginPlay();

	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	SkillManagerRef = GamemodeRef.Get()->GetGlobalSkillManagerBaseRef();
}

void ASkillBase::InitSkill(FSkillInfoStruct NewSkillInfo)
{
	SkillInfo=NewSkillInfo;
	SkillInfo.bHidenInGame = true; 
	AssetManagerBaseRef =GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();

	//If SkillActor's life span is sync with causer, then destroy with causer 
	if (SkillInfo.bSkillLifeSpanWithCauser)
	{
		SkillInfo.Causer->OnCharacterDied.AddDynamic(this, &ASkillBase::DestroySkill);
	}
}

void ASkillBase::ActivateSkill_Implementation()
{
	SetActorHiddenInGame(false);
	UseSkillGauge();
}

void ASkillBase::DeactivateSkill()
{
	FTransform skillTransform;
	skillTransform.SetLocation({0,0,-400});
	SetActorTransform(skillTransform);
	if (SkillInfo.SkillGradeStruct.bIsGradeValid)
	{
		SkillGrade = ESkillGrade::E_None;
		VariableMap.Empty();
	}
	SkillInfo.bHidenInGame = true;
	SetActorHiddenInGame(true);
}

void ASkillBase::DestroySkill()
{
	checkf(IsValid(SkillManagerRef.Get()), TEXT("Failed to get SkillBase class"));
	SkillManagerRef.Get()->RemoveSkillInSkillBaseMap(SkillInfo.Causer.Get());
	Destroy();
}

bool ASkillBase::CheckSkillGauge()
{
	float currSkillGauge = SkillInfo.Causer->GetCharacterState().CharacterCurrSkillGauge;

	//Grade확인
	if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, SkillInfo.SkillGradeStruct.CommonGaugeReq, SkillInfo.SkillGradeStruct.RareGaugeReq, true, false))
	{
		SkillGrade = ESkillGrade::E_Common;
		VariableMap = SkillInfo.SkillGradeStruct.CommonVariableMap;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, SkillInfo.SkillGradeStruct.RareGaugeReq, SkillInfo.SkillGradeStruct.LegendGaugeReq, true, false))
	{
		SkillGrade = ESkillGrade::E_Rare;
		VariableMap = SkillInfo.SkillGradeStruct.RareVariableMap;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(currSkillGauge, SkillInfo.SkillGradeStruct.LegendGaugeReq, SkillInfo.SkillGradeStruct.MythicGaugeReq, true, false))
	{
		SkillGrade = ESkillGrade::E_Legend;
		VariableMap = SkillInfo.SkillGradeStruct.LegendVariableMap;
	}
	else if (currSkillGauge >= SkillInfo.SkillGradeStruct.MythicGaugeReq)
	{
		SkillGrade = ESkillGrade::E_Mythic;
		VariableMap = SkillInfo.SkillGradeStruct.MythicVariableMap;
	}
	else
	{
		SkillGrade = ESkillGrade::E_None;
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("스킬 게이지가 부족합니다.")), FColor::White);
		SkillInfo.Causer->ResetActionState();
		return false;
	}
	return true;
}

void ASkillBase::UseSkillGauge()
{
	if (SkillInfo.SkillGradeStruct.bIsGradeValid)
	{
		FCharacterStateStruct newState = SkillInfo.Causer->GetCharacterState();

		switch (SkillGrade)
		{
		case ESkillGrade::E_None:
			return;
		case ESkillGrade::E_Common:
			newState.CharacterCurrSkillGauge -= SkillInfo.SkillGradeStruct.CommonGaugeReq;
			break;
		case ESkillGrade::E_Rare:
			newState.CharacterCurrSkillGauge -= SkillInfo.SkillGradeStruct.RareGaugeReq;
			break;
		case ESkillGrade::E_Legend:
			newState.CharacterCurrSkillGauge -= SkillInfo.SkillGradeStruct.LegendGaugeReq;
			break;
		case ESkillGrade::E_Mythic:
			newState.CharacterCurrSkillGauge -= SkillInfo.SkillGradeStruct.MythicGaugeReq;
			break;
		}
		SkillInfo.Causer->UpdateCharacterState(newState);
	}
}

void ASkillBase::PlaySingleSound(FName SoundName)
{
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Skill, SkillInfo.SkillID, SoundName);
	}
}

float ASkillBase::PlayAnimMontage(ACharacter* Target, FName AnimName)
{
	FSkillAnimInfoStruct* skillAnimInfo = SkillInfo.SkillAssetStruct.AnimInfoMap.Find(AnimName);
	UAnimMontage* anim = skillAnimInfo->AnimMontage;
	float animPlayTime = Target->PlayAnimMontage(anim, skillAnimInfo->AnimPlayRate);
	return animPlayTime;
}

UNiagaraSystem* ASkillBase::GetNiagaraEffect(FName EffectName)
{
	if(!SkillInfo.SkillAssetStruct.EffectMap.Contains(EffectName)) return nullptr;
	return SkillInfo.SkillAssetStruct.EffectMap.Find(EffectName)->Effect;
}
