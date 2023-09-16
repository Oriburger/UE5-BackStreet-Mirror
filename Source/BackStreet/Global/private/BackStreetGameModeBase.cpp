// Copyright Epic Games, Inc. All Rights Reserved.

#include "../public/BackStreetGameModeBase.h"
#include "../public/DebuffManager.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
#include "../../StageSystem/public/StageData.h"
#include "../../Item/public/ProjectileBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/RangedWeaponBase.h"
#include "../../Item/public/MeleeWeaponBase.h"
#include "../../Item/public/ItemBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../public/AssetManagerBase.h"

ABackStreetGameModeBase::ABackStreetGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTable(TEXT("/Game/Character/EnemyCharacter/Data/D_EnemyStatDataTable.D_EnemyStatDataTable"));
	if (DataTable.Succeeded())
	{
		EnemyStatTable = DataTable.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> StageTypeDataTable(TEXT("/Game/System/StageManager/Data/D_StageEnemyTypeTable.D_StageEnemyTypeTable"));
	if (StageTypeDataTable.Succeeded())
	{
		StageTypeTable = StageTypeDataTable.Object;
	}
}

void ABackStreetGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void ABackStreetGameModeBase::InitializeGame()
{
	if (bIsInGame)
	{
		FActorSpawnParameters spawnParams;
		FRotator rotator;
		FVector spawnLocation = FVector::ZeroVector;

		CreateChapterManager();
		//ChapterManager = GetWorld()->SpawnActor<AChapterManagerBase>(AChapterManagerBase::StaticClass(), spawnLocation, rotator, spawnParams);
		//ChapterManager->SetOwner(this);
		//ChapterManager->CreateChapterManager();

		//------ 델리게이트 바인딩 ---------------
		FinishChapterDelegate.AddDynamic(this, &ABackStreetGameModeBase::FinishChapter);
		FadeOutDelegate.AddDynamic(this, &ABackStreetGameModeBase::FadeOut);


		//------ Ref 멤버 초기화  ---------------
		PlayerCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		//------ Global Buff/Debuff Manager 초기화 --------
		DebuffManager = NewObject<UDebuffManager>(this, UDebuffManager::StaticClass(), FName("BuffDebuffManager"));
		DebuffManager->InitDebuffManager(this);

	}
}

void ABackStreetGameModeBase::PlayCameraShakeEffect(ECameraShakeType EffectType, FVector Location, float Radius)
{
	if (!IsValid(GetWorld()) || !CameraShakeEffectList.IsValidIndex((uint8)EffectType)) return;
	if (!IsValid(PlayerCharacterRef) || PlayerCharacterRef->IsActorBeingDestroyed()) return;

	Location = Location + (PlayerCharacterRef->FollowingCamera->GetComponentLocation() - PlayerCharacterRef->GetActorLocation());
	UGameplayStatics::PlayWorldCameraShake(GetWorld(), CameraShakeEffectList[(uint8)EffectType], Location, Radius * 0.75f, Radius * 1.5f, 0.5f);
}

AItemBase* ABackStreetGameModeBase::SpawnItemToWorld(int32 ItemID, FVector SpawnLocation)
{
	if (!IsValid(GetWorld())) return nullptr;
	
	FActorSpawnParameters actorSpawnParameters;
	actorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ItemClass != nullptr)
	{
		AItemBase* newItem = Cast<AItemBase>(GetWorld()->SpawnActor(ItemClass, &SpawnLocation, nullptr, actorSpawnParameters));

		if (IsValid(newItem))
		{
			newItem->InitItem(ItemID);

			//임시코드!!!!!!!!!!!!!!!! 230704 @ljh
			if (IsValid(GetChapterManagerRef()) 
				&& GetChapterManagerRef()->GetCurrentStage())
			{
				GetChapterManagerRef()->GetCurrentStage()->AddItemList(newItem);
			}
		}
		return newItem;
	}
	return nullptr;
}

bool ABackStreetGameModeBase::IsLobbyStage()
{
	if (ChapterManager->GetCurrentStage() == ChapterManager->GetLobbyStage()) 
		return true; 
	else return false;
}

void ABackStreetGameModeBase::UpdateCharacterStat(ACharacterBase* TargetCharacter, FCharacterStatStruct NewStat)
{
	if (IsValid(TargetCharacter))
	{
		TargetCharacter->UpdateCharacterStat(NewStat);
	}
}

void ABackStreetGameModeBase::UpdateCharacterStatWithID(ACharacterBase* TargetCharacter, const uint32 CharacterID)
{
	if (IsValid(TargetCharacter) && TargetCharacter->ActorHasTag("Enemy"))
	{
		//DataTable로 부터 Read
		FString rowName = FString::FromInt(CharacterID);
		FEnemyStatStruct* typeInfo = EnemyStatTable->FindRow<FEnemyStatStruct>(FName(rowName), rowName);

		if (typeInfo)
		{
			FCharacterStatStruct NewStat;
			NewStat.CharacterMaxHP = typeInfo->CharacterMaxHP;
			NewStat.CharacterDefense = typeInfo->CharacterDefense;
			NewStat.CharacterAtkSpeed = typeInfo->CharacterAtkSpeed;
			NewStat.CharacterAtkMultiplier = typeInfo->CharacterAtkMultiplier;
			NewStat.CharacterMoveSpeed = typeInfo->CharacterMoveSpeed;

			TargetCharacter->UpdateCharacterStat(NewStat);
		}
	}
}

void ABackStreetGameModeBase::UpdateProjectileStatWithID(AProjectileBase* TargetProjectile, const int32 ProjectileID)
{
	if (IsValid(TargetProjectile) && IsValid(ProjectileStatTable))
	{
		FString rowName = FString::FromInt(ProjectileID);
		FProjectileStatStruct* newStat = ProjectileStatTable->FindRow<FProjectileStatStruct>(FName(rowName), rowName);
		if (newStat != nullptr)
		{
			TargetProjectile->UpdateProjectileStat(*newStat);
		}
	}
}

FStageEnemyTypeStruct ABackStreetGameModeBase::GetStageTypeInfoWithRow(uint16 row)
{
	FString rowName = FString::FromInt(row);
	FStageEnemyTypeStruct* newStat = StageTypeTable->FindRow<FStageEnemyTypeStruct>(FName(rowName), rowName);
	if (newStat == nullptr) return FStageEnemyTypeStruct();
	return *newStat;
}

FCharacterAnimAssetInfoStruct ABackStreetGameModeBase::GetCharacterAnimAssetInfoData(const int32 CharacterID)
{
	// 캐시된 데이터 확인
	if (CachedCharacterAssetInfoData.AnimAssetInfoMap.Contains(CharacterID))
	{
		return CachedCharacterAssetInfoData.AnimAssetInfoMap[CharacterID];
	}
	// AssetInfoTable에서 데이터 가져오기
	if (AnimAssetInfoTable)
	{
		FCharacterAnimAssetInfoStruct* assetInfoRow = AnimAssetInfoTable->FindRow<FCharacterAnimAssetInfoStruct>(FName(*FString::FromInt(CharacterID)), FString(""));
		if (assetInfoRow)
		{
			// 데이터를 캐시에 저장
			CachedCharacterAssetInfoData.AnimAssetInfoMap.Add(CharacterID, *assetInfoRow);
			return *assetInfoRow;
		}
	}
	// 데이터가 없을 경우 기본값 반환
	return FCharacterAnimAssetInfoStruct();
}

