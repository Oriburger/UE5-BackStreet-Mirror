// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/SkillBase.h"
#include "Components/AudioComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "../../Global/public/SkillManagerBase.h"
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
	checkf(assetInfoTableFinder.Succeeded(), TEXT("assetInfoTable 클래스 탐색에 실패했습니다."));
	SkillAssetInfoTable = assetInfoTableFinder.Object;
}

// Called when the game starts or when spawned
void ASkillBase::BeginPlay()
{
	Super::BeginPlay();
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
}

void ASkillBase::InitSkill_Implementation(AActor* Causer, class ACharacterBase* Target, int32 NewSkillID, ESkillGrade SkillGrade)
{
	//Stat, State 초기화 
	SkillID = NewSkillID;

	if (NewSkillID == 0)
	{
		SkillAssetInfo = FSkillAssetInfoStruct();
		SkillMesh->SetStaticMesh(nullptr);
		return;
	}

	//FSkillStatStruct newStat = GetSkillnStatInfoWithID(SkillID);
	//UpdateSkillStat(newStat);

	//에셋 초기화
	FSkillAssetInfoStruct newAssetInfo = GetSkillAssetInfoWithID(SkillID);
	SkillAssetInfo = newAssetInfo;

	if (SkillID != 0)
	{
		TArray<FSoftObjectPath> tempStream, assetToStream;
		tempStream.AddUnique(SkillAssetInfo.SkillMesh.ToSoftObjectPath());
		tempStream.AddUnique(SkillAssetInfo.EffectParticle.ToSoftObjectPath());
		tempStream.AddUnique(SkillAssetInfo.DestroyEffectParticle.ToSoftObjectPath());
		tempStream.AddUnique(SkillAssetInfo.SkillSound.ToSoftObjectPath());
		tempStream.AddUnique(SkillAssetInfo.SkillFailSound.ToSoftObjectPath());

		for (auto& assetPath : tempStream)
		{
			if (!assetPath.IsValid() || assetPath.IsNull()) continue;
			assetToStream.AddUnique(assetPath);
		}
		FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
		streamable.RequestAsyncLoad(assetToStream, FStreamableDelegate::CreateUObject(this, &ASkillBase::InitSkillAsset));
	}
}

void ASkillBase::SetSkillManagerRef(USkillManagerBase* NewSkillManager)
{
	SkillManagerRef = NewSkillManager;
}

void ASkillBase::InitSkillAsset()
{
	if (SkillAssetInfo.SkillMesh.IsValid())
	{
		SkillMesh->SetStaticMesh(SkillAssetInfo.SkillMesh.Get());
		SkillMesh->SetRelativeLocation(SkillAssetInfo.InitialLocation);
		SkillMesh->SetRelativeRotation(SkillAssetInfo.InitialRotation);
		SkillMesh->SetRelativeScale3D(SkillAssetInfo.InitialScale);
		SkillMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (SkillAssetInfo.DestroyEffectParticle.IsValid())
		DestroyEffectParticle = SkillAssetInfo.DestroyEffectParticle.Get();

	if (SkillAssetInfo.SkillSound.IsValid())
		SkillSound = SkillAssetInfo.SkillSound.Get();

	if (SkillAssetInfo.SkillFailSound.IsValid())
		SkillFailSound = SkillAssetInfo.SkillFailSound.Get();
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

void ASkillBase::SkillAttack() {}

void ASkillBase::StopSkillAttack() {}

void ASkillBase::PlayEffectSound(USoundCue* EffectSound)
{
	if (EffectSound == nullptr) return;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), EffectSound, GetActorLocation());
}

void ASkillBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(SkillTimerHandle);
}
