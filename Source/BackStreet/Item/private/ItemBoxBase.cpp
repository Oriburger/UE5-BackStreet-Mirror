// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/ItemBoxBase.h"
#include "../public/ItemBase.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Math/RandomStream.h"

// Sets default values
AItemBoxBase::AItemBoxBase()
{ 
	this->Tags.Add(FName("ItemBox"));
	PrimaryActorTick.bCanEverTick = false;
	SetActorTickEnabled(false);

	RootComponent = OverlapVolume = CreateDefaultSubobject<USphereComponent>("SPHERE_COLLISION");
	//OverlapVolume->SetupAttachment(RootComponent);
	OverlapVolume->SetRelativeScale3D(FVector(5.0f));
	OverlapVolume->SetCollisionProfileName("ItemTrigger", true);
		
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_MESH"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName("Item", false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	ParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ITEM_NIAGARA_COMPONENT"));
	ParticleComponent->SetupAttachment(RootComponent);

	//미션 아이템 명시적 추가
	SpawnItemTypeList.Add(EItemCategoryInfo::E_Mission);
	SpawnItemIDList.Add(0);
	ItemSpawnProbabilityList.Add(100.0f);
}

// Called when the game starts or when spawned
void AItemBoxBase::BeginPlay()
{
	Super::BeginPlay();

	OnPlayerOpenBegin.AddDynamic(this, &AItemBoxBase::OnItemBoxOpened);
	//OverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &AItemBoxBase::OnOverlapBegins);
	MeshComponent->OnComponentHit.AddDynamic(this, &AItemBoxBase::OnMeshHit);

	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
}

void AItemBoxBase::InitItemBox(bool _bIncludeMissionItem)
{	
	bIncludeMissionItem = _bIncludeMissionItem;
}

void AItemBoxBase::OnItemBoxOpened(AActor* Causer)
{
	if (!IsValid(Causer) || !Causer->ActorHasTag("Player")) return;
	if (MinSpawnItemCount > MaxSpawnItemCount) //정보 기입이 제대로 이뤄지지 않음
	{
		UE_LOG(LogTemp, Warning, TEXT("AItemBoxBase::OnItemBoxOpened) SpawnCount Info가 올바르지 않습니다."));
		return;
	}

	int32 spawnItemCount = UKismetMathLibrary::RandomIntegerInRange(MinSpawnItemCount, MaxSpawnItemCount);
	TArray<AItemBase*> spawnedItemList = SpawnItems(spawnItemCount);

	for (auto& itemRef : spawnedItemList)
	{
		if (!IsValid(itemRef)) continue;
		LaunchItem(itemRef);
	}
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OpenEffectParticle, GetActorLocation(), FRotator(), FVector(1.5f));
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), OpenEffectSound, GetActorLocation());
	Destroy();
}

void AItemBoxBase::OnOverlapBegins(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor->ActorHasTag("Player")) return;

	//UI Activate
}

void AItemBoxBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || OtherActor->ActorHasTag("Player")) return;

	//UI Deactivate
}

void AItemBoxBase::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	const FVector meshLocation = MeshComponent->GetComponentLocation();
	SetActorLocation(meshLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

TArray<AItemBase*> AItemBoxBase::SpawnItems(int32 TargetSpawnCount)
{
	//프로퍼티 내 각 배열 세팅이 올바르지 않은 경우
	if (SpawnItemTypeList.Num() != SpawnItemIDList.Num() || SpawnItemIDList.Num() != ItemSpawnProbabilityList.Num()) return TArray<AItemBase*>();

	TArray<AItemBase*> itemList; //반환할 아이템 리스트 
	TSet<int32> duplicateCheckSet; //중복 체크 Set

	bool bMissionItemFlag = !bIncludeMissionItem; //미션 아이템 스폰 체크, 아이템 박스 당 최대 1개
	int32 currentSpawnCount = 0;
	int32 currentTryCount = 0;

	while(currentSpawnCount < TargetSpawnCount)
	{
		if (++currentTryCount >= MAX_TRY_SPAWN_COUNT) break;

		int32 targetItemInfoIdx = !bMissionItemFlag ? 0 : UKismetMathLibrary::RandomIntegerInRange(1, SpawnItemTypeList.Num()-1);
		const int32 targetItemType = (int32)SpawnItemTypeList[targetItemInfoIdx];
		const int32 targetItemID = SpawnItemIDList[targetItemInfoIdx];
		const float targetItemProbability = ItemSpawnProbabilityList[targetItemInfoIdx];
		const int32 targetItemKey = targetItemType * 1000 + targetItemID; //Set에 넣어 중복 체크를 할때 사용할 고유 키 값.

		//이미 해당 아이템을 스폰했다면? 다시 선택
		if (duplicateCheckSet.Contains(targetItemKey)) continue;

		if (UKismetMathLibrary::RandomIntegerInRange(0, 100) <= targetItemProbability)
		{
			const float randomValue = UKismetMathLibrary::RandomFloatInRange(-10.0f, 10.0f);
			const FVector spawnLocation = GetActorLocation() + FVector(randomValue, randomValue, currentSpawnCount * SpawnLocationInterval + 125.0f);
			AItemBase* newItem = GamemodeRef.Get()->SpawnItemToWorld(targetItemType, targetItemID, spawnLocation);

			if (IsValid(newItem))
			{
				itemList.Add(newItem);
				newItem->SetActorScale3D(FVector(0.1));
				newItem->InitItem((EItemCategoryInfo)targetItemType, targetItemID);

				if (targetItemType == (int32)EItemCategoryInfo::E_Mission)
				{
					OnMissionItemSpawned.ExecuteIfBound(newItem); //새 아이템을 UMission에 등록
				}
				bMissionItemFlag = (bMissionItemFlag || (EItemCategoryInfo)targetItemType == EItemCategoryInfo::E_Mission);

				currentSpawnCount++;
				duplicateCheckSet.Add(targetItemKey); //성공적으로 스폰한 아이템은 셋에 넣어 보관
			}
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
