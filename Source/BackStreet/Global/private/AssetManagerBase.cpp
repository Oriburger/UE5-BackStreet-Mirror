
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


}

void AAssetManagerBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("Being Play"));
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
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
		return nullptr;
		break;
	}
	return enemy;
}

FName AAssetManagerBase::GetRandomMap()
{
	uint8 idx = FMath::RandRange(0, MapNames.Num() - 1);
	return MapNames[idx];
}





