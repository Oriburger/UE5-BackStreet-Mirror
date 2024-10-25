// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBoxBase.h"

#include "../Global/BackStreetGameModeBase.h"
#include "../System/MapSystem/NewChapterManagerBase.h"
#include "../System/MapSystem/StageManagerComponent.h"
#include "InteractiveCollisionComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Math/RandomStream.h"

// Sets default values
AItemBoxBase::AItemBoxBase()
{ 
	this->Tags.Add(FName("ItemBox"));
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AItemBoxBase::BeginPlay()
{
	Super::BeginPlay();
}


TArray<AItemBase*> AItemBoxBase::SpawnItems(int32 TargetSpawnCount)
{
	//������Ƽ �� �� �迭 ������ �ùٸ��� ���� ���
	if (!GamemodeRef.IsValid()) return{};
	if (SpawnItemIDList.Num() != ItemSpawnProbabilityList.Num()) return TArray<AItemBase*>();

	TArray<AItemBase*> itemList; //��ȯ�� ������ ����Ʈ 
	TSet<int32> duplicateCheckSet; //�ߺ� üũ Set

	int32 currentSpawnCount = 0;
	int32 currentTryCount = 0;

	while(currentSpawnCount < TargetSpawnCount)
	{
		if (++currentTryCount >= MAX_TRY_SPAWN_COUNT) break;

		int32 targetItemInfoIdx = UKismetMathLibrary::RandomIntegerInRange(1, SpawnItemIDList.Num()-1);
		const int32 targetItemID = SpawnItemIDList[targetItemInfoIdx];
		const float targetItemProbability = ItemSpawnProbabilityList[targetItemInfoIdx];
		const int32 targetItemKey = targetItemID; //Set�� �־� �ߺ� üũ�� �Ҷ� ����� ���� Ű ��.

		//�̹� �ش� �������� �����ߴٸ�? �ٽ� ����
		if (duplicateCheckSet.Contains(targetItemKey)) continue;

		const float randomValue = UKismetMathLibrary::RandomFloatInRange(-10.0f, 10.0f);
		const FVector spawnLocation = GetActorLocation() + FVector(randomValue, randomValue, currentSpawnCount * SpawnLocationInterval + 125.0f);
		AItemBase* newItem = GamemodeRef.Get()->SpawnItemToWorld(targetItemID, spawnLocation);

		if (IsValid(newItem))
		{
			itemList.Add(newItem);
			
			if (IsValid(GamemodeRef.Get()->GetChapterManagerRef()))
			{
				GamemodeRef.Get()->GetChapterManagerRef()->StageManagerComponent->RegisterActor(newItem);
			}

			currentSpawnCount++;
			duplicateCheckSet.Add(targetItemKey); //���������� ������ �������� �¿� �־� ����
		}
	}
	return itemList;
}

void AItemBoxBase::LaunchItem(AItemBase* TargetItem)
{
	if (!IsValid(TargetItem)) return;

	FVector itemLocation = TargetItem->GetActorLocation(); 
	FVector launchDirection = FMath::VRandCone(GetActorUpVector(), CornHalfRadius);
	//DrawDebugLine(GetWorld(), itemLocation, itemLocation + launchDirection * 100.0f, FColor::MakeRandomColor(), false, 20.0f, 0, 5.0f);

	TargetItem->SetLaunchDirection(launchDirection * LaunchSpeed);
	TargetItem->ActivateProjectileMovement();
	TargetItem->ActivateItem();
}
