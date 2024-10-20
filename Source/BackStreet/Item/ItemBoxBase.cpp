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
	//프로퍼티 내 각 배열 세팅이 올바르지 않은 경우
	if (!GamemodeRef.IsValid()) return{};
	if (SpawnItemIDList.Num() != ItemSpawnProbabilityList.Num()) return TArray<AItemBase*>();

	TArray<AItemBase*> itemList; //반환할 아이템 리스트 
	TSet<int32> duplicateCheckSet; //중복 체크 Set

	int32 currentSpawnCount = 0;
	int32 currentTryCount = 0;

	while(currentSpawnCount < TargetSpawnCount)
	{
		if (++currentTryCount >= MAX_TRY_SPAWN_COUNT) break;

		int32 targetItemInfoIdx = UKismetMathLibrary::RandomIntegerInRange(1, SpawnItemIDList.Num()-1);
		const int32 targetItemID = SpawnItemIDList[targetItemInfoIdx];
		const float targetItemProbability = ItemSpawnProbabilityList[targetItemInfoIdx];
		const int32 targetItemKey = targetItemID; //Set에 넣어 중복 체크를 할때 사용할 고유 키 값.

		//이미 해당 아이템을 스폰했다면? 다시 선택
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
			duplicateCheckSet.Add(targetItemKey); //성공적으로 스폰한 아이템은 셋에 넣어 보관
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
