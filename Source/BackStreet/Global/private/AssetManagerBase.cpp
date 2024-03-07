
#include "../public/AssetManagerBase.h"
#include "../../Item/public/ItemBase.h"
#include "../../Item/public/ProjectileBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../public/BackStreetGameModeBase.h"
#include "UObject/ConstructorHelpers.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "../../Item/public/RewardBoxBase.h"

AAssetManagerBase::AAssetManagerBase()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AAssetManagerBase::BeginPlay()
{
	Super::BeginPlay();
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	//------ Sound Datatable Initiation ---------
	static ConstructorHelpers::FObjectFinder<UDataTable> systemSoundAssetTableFinder(TEXT("/Game/Asset/Data/D_SystemSoundAsset.D_SystemSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> weaponSoundAssetTableFinder(TEXT("/Game/Asset/Data/D_WeaponSoundAsset.D_WeaponSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> characterSoundAssetTableFinder(TEXT("/Game/Asset/Data/D_CharacterSoundAsset.D_CharacterSoundAsset"));
	static ConstructorHelpers::FObjectFinder<UDataTable> skillSoundAssetTableFinder(TEXT("/Game/Asset/Data/D_SkillSoundAsset.D_SkillSoundAsset"));
	
	checkf(systemSoundAssetTableFinder.Succeeded(), TEXT("SystemSoundAssetTable class discovery failed."));
	checkf(weaponSoundAssetTableFinder.Succeeded(), TEXT("WeaponSoundAssetTable class discovery failed."));
	checkf(characterSoundAssetTableFinder.Succeeded(), TEXT("CharacterSoundAssetTable class discovery failed."));
	checkf(skillSoundAssetTableFinder.Succeeded(), TEXT("SkillSoundAssetTable class discovery failed."));
	
	SystemSoundAssetTable = systemSoundAssetTableFinder.Object;
	WeaponSoundAssetTable = weaponSoundAssetTableFinder.Object;
	CharacterSoundAssetTable = characterSoundAssetTableFinder.Object;
	SkillSoundAssetTable = skillSoundAssetTableFinder.Object;
	
}

TSubclassOf<AEnemyCharacterBase> AAssetManagerBase::GetEnemyWithID(int32 EnemyID)
{
	TSubclassOf<AEnemyCharacterBase> enemy;

	switch (EnemyID)
	{
	case 1001:
		enemy = EnemyAssets[0];
		break;
	case 1002:
		enemy = EnemyAssets[0];
		break;
	case 1003:
		enemy = EnemyAssets[2];
		break;
	case 1100:
		enemy = EnemyAssets[1];
		break;
	case 1101:
		enemy = EnemyAssets[3];
		break;
	case 1102:
		enemy = EnemyAssets[3];
		break;
	case 1200:
		enemy = EnemyAssets[4];
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("Wrong ID"));
		check(0);
	}
	return enemy;
}

FName AAssetManagerBase::GetRandomMap()
{
	uint8 idx = FMath::RandRange(0, MapNames.Num() - 1);
	return MapNames[idx];
}

TMap<FName, FSoundArrayContainer> AAssetManagerBase::GetSystemSoundMapWithID(int32 NewID)
{	
	// Read from dataTable
	FString rowName = FString::FromInt(NewID);
	FSoundAssetInfoStruct* SoundAsset = SystemSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	TMap<FName, FSoundArrayContainer> SoundMap = SoundAsset->SoundMap;
	
	return SoundMap;	
}

TMap<FName, FSoundArrayContainer> AAssetManagerBase::GetWeaponSoundMapWithID(int32 NewID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(NewID);
	FSoundAssetInfoStruct* SoundAsset = WeaponSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	TMap<FName, FSoundArrayContainer> SoundMap = SoundAsset->SoundMap;

	return SoundMap;
}

TMap<FName, FSoundArrayContainer> AAssetManagerBase::GetCharacterSoundMapWithID(int32 NewID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(NewID);
	FSoundAssetInfoStruct* SoundAsset = CharacterSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	TMap<FName, FSoundArrayContainer> SoundMap = SoundAsset->SoundMap;

	return SoundMap;
}

TMap<FName, FSoundArrayContainer> AAssetManagerBase::GetSkillSoundMapWithID(int32 NewID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(NewID);
	FSoundAssetInfoStruct* SoundAsset = SkillSoundAssetTable->FindRow<FSoundAssetInfoStruct>(FName(rowName), rowName);

	TMap<FName, FSoundArrayContainer> SoundMap = SoundAsset->SoundMap;

	return SoundMap;
}








