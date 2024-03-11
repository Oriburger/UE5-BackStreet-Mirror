// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/SkillBase.h"
#include "Components/AudioComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "../../Global/public/SkillManagerBase.h"
#include "../../Global/public/AssetManagerBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"

// Sets default values
ASkillBase::ASkillBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	DefaultSceneRoot->SetupAttachment(RootComponent);
	SetRootComponent(DefaultSceneRoot);

	SkillMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skill_MESH"));
	SkillMesh->SetupAttachment(DefaultSceneRoot);
	
	static ConstructorHelpers::FObjectFinder<UDataTable> skillAssetInfoTableFinder(TEXT("/Game/Skill/Data/D_SkillAssetInfo.D_SkillAssetInfo"));
	checkf(skillAssetInfoTableFinder.Succeeded(), TEXT("SkillAssetInfoTable class discovery failed."));
	SkillAssetInfoTable = skillAssetInfoTableFinder.Object;
}

// Called when the game starts or when spawned
void ASkillBase::BeginPlay()
{
	Super::BeginPlay();
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}

void ASkillBase::InitSkill_Implementation(AActor* NewCauser, TArray<class ACharacterBase*>& NewTargetList, int32 NewSkillID, float NewSkillStartTiming)
{
	TargetList.Empty();
	Causer = NewCauser;
	TargetList = NewTargetList;
	SkillID = NewSkillID;

	AssetManagerBaseRef = GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();

	//Reset Asset
	InitAsset(SkillID);

	SkillInfo = GetSkillInfoWithID(SkillID);
	switch (GetSkillGrade())
	{
			case ESkillGrade::E_Common:
				if (!SkillInfo.SkillCommonVariableMap.IsEmpty())
				{
					SkillGradeVariableMap = SkillInfo.SkillCommonVariableMap;
				}
				break;

			case ESkillGrade::E_Rare:
				if (!SkillInfo.SkillRareVariableMap.IsEmpty())
				{
					SkillGradeVariableMap = SkillInfo.SkillRareVariableMap;
				}
				break;

			case ESkillGrade::E_Legend:
				if (!SkillInfo.SkillLegendVariableMap.IsEmpty())
				{
					SkillGradeVariableMap = SkillInfo.SkillLegendVariableMap;
				}
				break;

			case ESkillGrade::E_Mythic:
				if (!SkillInfo.SkillMythicVariableMap.IsEmpty())
				{
					SkillGradeVariableMap = SkillInfo.SkillMythicVariableMap;
				}
				break;
			
			default:
				break;
	}
	GetWorld()->GetTimerManager().SetTimer(SkillStartTimingTimerHandle, this, &ASkillBase::TrySkill, NewSkillStartTiming, false);
}

void ASkillBase::TrySkill()
{
	this->SetActorHiddenInGame(false);
	StartSkill();
	GetWorldTimerManager().ClearTimer(SkillStartTimingTimerHandle);
}

void ASkillBase::InitAsset(int32 NewSkillID) 
{
	SkillAssetInfo = GetSkillAssetInfoWithID(SkillID);
	TArray<FSoftObjectPath> AssetToStream;

	// Mesh
	AssetToStream.AddUnique(SkillAssetInfo.SkillActorMesh.ToSoftObjectPath());

	//VFX
	if (!SkillAssetInfo.EffectParticleList.IsEmpty())
	{
		for (int32 i = 0; i < SkillAssetInfo.EffectParticleList.Num(); i++)
		{
			AssetToStream.AddUnique(SkillAssetInfo.EffectParticleList[i].ToSoftObjectPath());
		}
	}

	AssetToStream.AddUnique(SkillAssetInfo.DestroyEffectParticle.ToSoftObjectPath());

	//Image
	AssetToStream.AddUnique(SkillAssetInfo.SkillIconImage.ToSoftObjectPath());

	FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
	streamable.RequestAsyncLoad(AssetToStream, FStreamableDelegate::CreateUObject(this, &ASkillBase::SetAsset));
}

void ASkillBase::SetAsset()
{
	//VFX
	if (!SkillAssetInfo.EffectParticleList.IsEmpty())
	{
		for (TSoftObjectPtr<UNiagaraSystem> effect : SkillAssetInfo.EffectParticleList)
		{
			if (effect.IsValid())
				SkillEffectParticleList.AddUnique(effect.Get());
		}
	}

	SkillDestroyEffectParticle = SkillAssetInfo.DestroyEffectParticle.Get();
	
	//Image
	SkillIconImage = SkillAssetInfo.SkillIconImage.Get();
}

void ASkillBase::HideSkill()
{
	class ACharacterBase* causer = Cast<class ACharacterBase>(Causer);
	if (!IsValid(causer) || !IsValid(causer->GetCurrentWeaponRef())) return;

	FTransform skillTransform;
	FVector skillLocation;
	skillLocation.Set(0, 0, -400);
	skillTransform.SetLocation(skillLocation);
	this->SetActorTransform(skillTransform);
	this->SetActorHiddenInGame(true);
}

void ASkillBase::SetSkillManagerRef(USkillManagerBase* NewSkillManager)
{
	SkillManagerRef = NewSkillManager;
}

FSkillInfoStruct ASkillBase::GetSkillInfoWithID(int32 TargetSkillID)
{
	if(SkillManagerRef == nullptr) return FSkillInfoStruct();

	UDataTable * skillInfoTable = SkillManagerRef->GetSkillInfoTable();
	if (skillInfoTable != nullptr && TargetSkillID != 0)
	{
		FSkillInfoStruct* newInfo;
		FString rowName = FString::FromInt(TargetSkillID);

		newInfo = skillInfoTable->FindRow<FSkillInfoStruct>(FName(rowName), rowName);
		if (newInfo != nullptr) return *newInfo;
	}
	return FSkillInfoStruct();
}

FSkillAssetInfoStruct ASkillBase::GetSkillAssetInfoWithID(int32 TargetSkillID)
{
	if (SkillAssetInfoTable != nullptr && TargetSkillID != 0)
	{
		FSkillAssetInfoStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetSkillID);

		newInfo = SkillAssetInfoTable->FindRow<FSkillAssetInfoStruct>(FName(rowName), rowName);
		if (newInfo != nullptr) return *newInfo;
	}
	return FSkillAssetInfoStruct();
}

void ASkillBase::PlaySingleSound(FName SoundName)
{
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Skill, SkillID, SoundName);
	}
}

void ASkillBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(SkillStartTimingTimerHandle);
}

ESkillGrade ASkillBase::GetSkillGrade()
{
	return Cast<ACharacterBase> (Causer)->GetCurrentWeaponRef()->GetWeaponStat().SkillSetInfo.SkillGrade;
}