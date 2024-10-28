// Fill out your copyright notice in the Description page of Project Settings.
#include "SkillBase.h"
#include "../AssetSystem/AssetManagerBase.h"
#include "../../Character/Component/SkillManagerComponentBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/CharacterBase.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASkillBase::ASkillBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	DefaultSceneRoot->SetupAttachment(RootComponent);
	SetRootComponent(DefaultSceneRoot);
}

void ASkillBase::BeginPlay()
{
	Super::BeginPlay();

	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	AssetManagerBaseRef = GameModeRef.Get()->GetGlobalAssetManagerBaseRef();
	SkillStatTable = GameModeRef.Get()->SkillStatTable;
}

void ASkillBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (SkillState.SkillUpgradeInfoMap.Contains(ESkillUpgradeType::E_CoolTime))
	{
		//UE_LOG(LogTemp, Warning, TEXT("SKILLBASE CoolTime : %.2lf / bIsBlocked : %d"), SkillState.SkillUpgradeInfoMap[ESkillUpgradeType::E_CoolTime].Variable, (int32)SkillState.bIsBlocked)
	}
}

void ASkillBase::InitSkill(FSkillStatStruct NewSkillStat, USkillManagerComponentBase* NewSkillManagerComponent)
{
	SkillStat = NewSkillStat;
	SkillManagerComponentRef = NewSkillManagerComponent;
	FString rowName = FString::FromInt(SkillStat.SkillID);
	FSkillStatStruct* skillStat = SkillStatTable->FindRow<FSkillStatStruct>(FName(rowName), rowName);
	checkf(skillStat != nullptr, TEXT("SkillStat is not valid"));
	SkillStat = *skillStat;

	SkillState.bIsHidden = true;
	SkillState.bIsStateValid = true;
	TArray<ESkillUpgradeType> upgradeTypeList;
	SkillStat.LevelInfo.GenerateKeyArray(upgradeTypeList);
	for (ESkillUpgradeType& upgradeType : upgradeTypeList)
	{
		if (SkillStat.LevelInfo[upgradeType].SkillLevelInfoList.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("ASkillBase::InitSkill, ID %d) SkillLevelInfoList is empty"), NewSkillStat.SkillID);
		}
		FSkillUpgradeLevelInfo newUpgradeInfo = SkillStat.LevelInfo[upgradeType].SkillLevelInfoList[0];
		SkillState.SkillUpgradeInfoMap.Add(upgradeType, newUpgradeInfo);
	}

	OwnerCharacterBaseRef = Cast<ACharacterBase>(SkillManagerComponentRef->GetOwner());
	//If SkillActor's life span is sync with causer, then destroy with causer 
	if (SkillStat.bIsLifeSpanWithCauser)
	{
		OwnerCharacterBaseRef->OnDeath.AddUObject(this, &ASkillBase::DestroySkill);
	}
}

void ASkillBase::ActivateSkill_Implementation()
{
	if (!SkillState.SkillUpgradeInfoMap.Contains(ESkillUpgradeType::E_CoolTime)) return;
	SetActorHiddenInGame(false);
	
	//Set cooltime timer
	FSkillUpgradeLevelInfo coolTimeInfo = SkillState.SkillUpgradeInfoMap[ESkillUpgradeType::E_CoolTime];
	if (coolTimeInfo.Variable > 0.0f)
	{
		SetSkillBlockedTimer(coolTimeInfo.Variable);
	}
}

void ASkillBase::DeactivateSkill()
{
	FTransform skillTransform;
	skillTransform.SetLocation({0,0,-400});
	SetActorTransform(skillTransform);
	SkillState.bIsHidden = true;
	SetActorHiddenInGame(true);
}

void ASkillBase::DestroySkill()
{
	if(IsValid(SkillManagerComponentRef.Get()))
	{
		SkillManagerComponentRef.Get()->RemoveSkill(SkillStat.SkillID);
	}
	Destroy();
}

ACharacterBase* ASkillBase::GetOwnerCharacterRef()
{
	if(!IsValid(OwnerCharacterBaseRef.Get())) return nullptr;
	return OwnerCharacterBaseRef.Get();
}

void ASkillBase::PlaySingleSound(FName SoundName)
{
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Skill, SkillStat.SkillID, SoundName);
	}
}

float ASkillBase::PlayAnimMontage(ACharacter* Target, FName AnimName)
{
	if(!SkillStat.SkillAssetStruct.AnimInfoMap.Contains(AnimName)) return 0.0f;
	FSkillAnimInfoStruct* skillAnimInfo = SkillStat.SkillAssetStruct.AnimInfoMap.Find(AnimName);
	checkf(skillAnimInfo != nullptr, TEXT("SkillAnimAsset is not valid"));
	UAnimMontage* anim = skillAnimInfo->AnimMontage;
	if (!IsValid(anim)) return 0.0f;
	float animPlayTime = Target->PlayAnimMontage(anim, skillAnimInfo->AnimPlayRate);
	return animPlayTime;
}

UNiagaraSystem* ASkillBase::GetNiagaraEffect(FName EffectName)
{
	if(!SkillStat.SkillAssetStruct.EffectMap.Contains(EffectName)) return nullptr;
	return SkillStat.SkillAssetStruct.EffectMap.Find(EffectName)->Effect;
}

float ASkillBase::GetSkillRemainingCoolTime()
{
	//If timer is invalid, return -1.0f
	return GetWorldTimerManager().GetTimerRemaining(SkillCoolTimeHandle);
}

void ASkillBase::SetSkillBlockedTimer(float Delay)
{
	//Clear timer resource
	GetWorldTimerManager().ClearTimer(SkillCoolTimeHandle);
	SkillCoolTimeHandle.Invalidate();

	//Set new timer that is going to call the function "ResetSkillBlockdTimer"
	SkillState.bIsBlocked = true;
	GetWorldTimerManager().SetTimer(SkillCoolTimeHandle, this, &ASkillBase::ResetSkillBlockedTimer
									, Delay, false);
}

void ASkillBase::ResetSkillBlockedTimer()
{
	if (!IsValid(SkillManagerComponentRef->GetOwner())) return;
	SkillState.bIsBlocked = false;
}