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
	
	static ConstructorHelpers::FObjectFinder<UDataTable> assetInfoTableFinder(TEXT("/Game/Skill/Data/D_SkillAssetInfo.D_SkillAssetInfo"));
	checkf(assetInfoTableFinder.Succeeded(), TEXT("AssetInfoTable class discovery failed."));
	SkillAssetInfoTable = assetInfoTableFinder.Object;
}

// Called when the game starts or when spawned
void ASkillBase::BeginPlay()
{
	Super::BeginPlay();
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}

void ASkillBase::InitSkill(AActor* NewCauser, TArray<class ACharacterBase*>& NewTargetList, int32 NewSkillID, float NewSkillStartTiming)
{
	TargetList.Empty();
	Causer = NewCauser;
	TargetList = NewTargetList;
	SkillID = NewSkillID;

	//Reset Asset
	InitAsset(SkillID);

	GetWorld()->GetTimerManager().SetTimer(SkillStartTimingTimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			this->SetActorHiddenInGame(false);
			StartSkill();
			GetWorldTimerManager().ClearTimer(SkillStartTimingTimerHandle);
		}), NewSkillStartTiming, false);
}

void ASkillBase::InitAsset(int32 NewSkillID) 
{
	SkillAssetInfo = GetSkillAssetInfoWithID(SkillID);
	TArray<FSoftObjectPath> AssetToStream;

	// Mesh
	AssetToStream.AddUnique(SkillAssetInfo.SkillActorMesh.ToSoftObjectPath());

	//SFX
	if (!SkillAssetInfo.SkillSoundList.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("#"));
		for (int32 i = 0; i < SkillAssetInfo.SkillSoundList.Num(); i++)
		{
			AssetToStream.AddUnique(SkillAssetInfo.SkillSoundList[i].ToSoftObjectPath());
		}
	}

	//VFX
	if (!SkillAssetInfo.EffectParticleList.IsEmpty())
	{
		for (int32 i = 0; i < SkillAssetInfo.SkillSoundList.Num(); i++)
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
	//SFX
	if (!SkillAssetInfo.SkillSoundList.IsEmpty())
	{
		for (TSoftObjectPtr<USoundCue> sound : SkillAssetInfo.SkillSoundList)
		{
			if (sound.IsValid())
				SkillAssetInfo.SkillSoundList.AddUnique(sound.Get());
		}
	}

	//VFX
	if (!SkillAssetInfo.EffectParticleList.IsEmpty())
	{
		for (TSoftObjectPtr<UNiagaraSystem> effect : SkillAssetInfo.EffectParticleList)
		{
			if (effect.IsValid())
				SkillAssetInfo.EffectParticleList.AddUnique(effect.Get());
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

void ASkillBase::PlayEffectSound(USoundCue* EffectSound)
{
	if (EffectSound == nullptr) return;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), EffectSound, GetActorLocation());
}

void ASkillBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(SkillStartTimingTimerHandle);
}