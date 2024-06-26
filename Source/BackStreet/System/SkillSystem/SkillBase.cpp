// Fill out your copyright notice in the Description page of Project Settings.
#include "SkillBase.h"
#include "SkillManagerBase.h"
#include "../AssetSystem/AssetManagerBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/CharacterBase.h"
#include "../../Item/Weapon/WeaponBase.h"
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

// Called when the game starts or when spawned
void ASkillBase::BeginPlay()
{
	Super::BeginPlay();

	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	SkillManagerRef = GamemodeRef.Get()->GetGlobalSkillManagerBaseRef();
}

void ASkillBase::InitSkill(FSkillInfoStruct NewSkillInfo)
{
	SkillInfo = NewSkillInfo;
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
	
	//Set cooltime timer
	FSkillLevelStruct skillLevelInfo = SkillInfo.SkillLevelStruct;
	if (skillLevelInfo.bIsLevelValid && skillLevelInfo.CoolTime > 0.0f)
	{
		SetSkillBlockedTimer(skillLevelInfo.CoolTime);
	}
}

void ASkillBase::DeactivateSkill()
{
	FTransform skillTransform;
	skillTransform.SetLocation({0,0,-400});
	SetActorTransform(skillTransform);
	SkillInfo.bHidenInGame = true;
	SetActorHiddenInGame(true);
}

void ASkillBase::DestroySkill()
{
	checkf(IsValid(SkillManagerRef.Get()), TEXT("Failed to get SkillBase class"));
	SkillManagerRef.Get()->RemoveSkillInSkillBaseMap(SkillInfo.Causer.Get());
	Destroy();
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
	SkillManagerRef.Get()->SetSkillBlockState(SkillInfo.Causer.Get(), SkillInfo.SkillType, true);
	GetWorldTimerManager().SetTimer(SkillCoolTimeHandle, this, &ASkillBase::ResetSkillBlockedTimer
									, Delay, false);
}

void ASkillBase::ResetSkillBlockedTimer()
{
	if (!SkillInfo.Causer.IsValid()) return;
	SkillManagerRef.Get()->SetSkillBlockState(SkillInfo.Causer.Get(), SkillInfo.SkillType, false);
}