// Fill out your copyright notice in the Description page of Project Settings.
#include "SkillBase.h"
#include "../AssetSystem/AssetManagerBase.h"
#include "../../Character/Component/SkillManagerComponentBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/CharacterBase.h"
#include "../AISystem/AIControllerBase.h"
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
}

void ASkillBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASkillBase::InitSkill(FSkillStatStruct NewSkillStat, USkillManagerComponentBase* NewSkillManagerComponent, FSkillInfo NewSkillInfo)
{
	SkillInfo = NewSkillInfo;
	SkillManagerComponentRef = NewSkillManagerComponent;
	
	OwnerCharacterBaseRef = Cast<ACharacterBase>(SkillManagerComponentRef->GetOwner());
	
	//If SkillActor's life span is sync with causer, then destroy with causer 
	if (SkillInfo.bIsLifeSpanWithCauser)
	{
		OwnerCharacterBaseRef->OnDeath.AddUObject(this, &ASkillBase::DeactivateSkill);
	}
}

void ASkillBase::ActivateSkill_Implementation()
{

}

void ASkillBase::DeactivateSkill()
{
	Destroy();
}

ACharacterBase* ASkillBase::GetOwnerCharacterRef()
{
	if(!IsValid(OwnerCharacterBaseRef.Get())) return nullptr;
	return OwnerCharacterBaseRef.Get();
}

float ASkillBase::PlayAnimMontage(ACharacter* Target, FName AnimName)
{
	UE_LOG(LogTemp, Error, TEXT("ASkillBase::PlayAnimMontage is LEGACY. Remove this code."));
	return 0.0f;
}