// Fill out your copyright notice in the Description page of Project Settings.
#include "ItemBase.h"
#include "../Global/BackStreetGameModeBase.h"
#include "../Character/CharacterBase.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "../Character/Component/ItemInventoryComponent.h"
#include "InteractiveCollisionComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/WidgetComponent.h"

#define DEFAULT_ITEM_LAUNCH_SPEED 500.0f
#define ITEM_WEAPON_ID_DIFF_VALUE 20000 
#define ITEM_BULLET_ID_DIFF_VALUE 20001

// Sets default values
AItemBase::AItemBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	this->Tags.Add(FName("Item"));

	PrimaryActorTick.bCanEverTick = false;
	//DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	//DefaultSceneRoot->SetupAttachment(RootComponent);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_STATIC_MESH"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName("Item", true);
	SetRootComponent(MeshComponent);

	IconWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ITEM_INFO_WIDGET"));
	IconWidgetComponent->SetupAttachment(MeshComponent);
	IconWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	IconWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IconWidgetComponent->SetVisibility(false);

	ParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ITEM_NIAGARA_COMPONENT"));
	ParticleComponent->SetupAttachment(MeshComponent);
	ParticleComponent->bAutoActivate = false;

	ItemTriggerVolume = CreateDefaultSubobject<UInteractiveCollisionComponent>("SPHERE_COLLISION");
	ItemTriggerVolume->SetupAttachment(RootComponent);
	ItemTriggerVolume->SetWorldScale3D(FVector(5.0f));
	ItemTriggerVolume->SetCollisionProfileName("ItemTrigger", true);
	ItemTriggerVolume->SetBoxExtent(FVector(40.0f), false);
	ItemTriggerVolume->OnComponentBeginOverlap.AddUniqueDynamic(this, &AItemBase::OnOverlapBegins);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PROJECTILE_MOVEMENT"));
	ProjectileMovement->InitialSpeed = DEFAULT_ITEM_LAUNCH_SPEED;
	ProjectileMovement->MaxSpeed = DEFAULT_ITEM_LAUNCH_SPEED;
	ProjectileMovement->bAutoActivate = false;	
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.5f;
	ProjectileMovement->Friction = 0.6f;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 2.0f;	
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
	ItemDataInfoTable = GamemodeRef.Get()->ItemInfoTable;

	OnPlayerBeginPickUp.BindUFunction(this, FName("OnItemPicked"));
	ItemTriggerVolume->OnInteractionBegin.AddDynamic(this, &AItemBase::OnItemPicked);
	PlayerRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	ActivateItem();
}

void AItemBase::InitItem(int32 NewItemID, FItemInfoDataStruct InfoOverride)
{
	ItemInfo = InfoOverride.ItemID == 0 ? GetItemInfoWithID(NewItemID) : InfoOverride;

	if (ItemInfo.ItemID == 0) return;

	if ((InfoOverride.ItemID == 0 && ItemInfo.ItemType != EItemCategoryInfo::E_SubWeapon)
		|| ItemInfo.ItemType == EItemCategoryInfo::E_Craft)
	{
		ItemInfo.ItemAmount = 1;
	}

	bool bNeedToAsyncLoad = false;
	TArray<FSoftObjectPath> assetToStream;
	if (!ItemInfo.ItemEffect.IsNull())
	{
		bNeedToAsyncLoad = !IsValid(ItemInfo.ItemEffect.Get());
		assetToStream.AddUnique(ItemInfo.ItemEffect.ToSoftObjectPath());
	}

	if (!ItemInfo.ItemMesh.IsNull())
	{
		bNeedToAsyncLoad = bNeedToAsyncLoad || !IsValid(ItemInfo.ItemMesh.Get());
		assetToStream.AddUnique(ItemInfo.ItemMesh.ToSoftObjectPath());
		assetToStream.AddUnique(ItemInfo.OutlineMaterial.ToSoftObjectPath());
	}

	//비동기 에셋 로드가 필요한지 유무 체크
	if(bNeedToAsyncLoad)
	{
		FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
		streamable.RequestAsyncLoad(assetToStream, FStreamableDelegate::CreateUObject(this, &AItemBase::InitializeItemMesh));
	}
	else
	{
		InitializeItemMesh();
	}

	OnItemInitialized.Broadcast(ItemInfo);

	UE_LOG(LogTemp, Warning, TEXT("AItemBase::InitItem()"));
	UE_LOG(LogTemp, Warning, TEXT("ItemID : %d"), ItemInfo.ItemID);
	UE_LOG(LogTemp, Warning, TEXT("ItemImage : %s"), *UKismetSystemLibrary::GetDisplayName(ItemInfo.ItemImage));
	UE_LOG(LogTemp, Warning, TEXT("ItemType : %d"), (int32)ItemInfo.ItemType);
	UE_LOG(LogTemp, Warning, TEXT("ItemAmount : %d"), ItemInfo.ItemAmount);
}

void AItemBase::SetItemAmount(int32 NewAmount)
{
	if (ItemInfo.ItemType != EItemCategoryInfo::E_SubWeapon && ItemInfo.ItemType != EItemCategoryInfo::E_Craft) return;
	ItemInfo.ItemAmount = NewAmount;
}

void AItemBase::AdjustItemZLocation(float ZOffsetOverride, bool bDrawDebugLine)
{
	FHitResult hitResult;

	// 여러 오브젝트 타입을 대상으로 설정 (WorldStatic, WorldDynamic)
	TArray<TEnumAsByte<EObjectTypeQuery>> objectQueryParams;
	objectQueryParams.Add(EObjectTypeQuery::ObjectTypeQuery1); // WorldStatic
	objectQueryParams.Add(EObjectTypeQuery::ObjectTypeQuery2); // WorldDynamic

	FVector startLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FVector endLocation = GetActorLocation() + GetActorLocation().UpVector * 200.0f;
	FVector resultLocation;

	// 천장 체크
	// WorldStatic, WorldDynamic로 트레이스
	// 결과를 startLocation에 저장
	// 라인트레이스 수행
	bool bHit = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), startLocation, endLocation, objectQueryParams
		, true, { this }, bDrawDebugLine ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitResult, true, FLinearColor::Yellow, FLinearColor::Green, 20.0f
	);

	startLocation = (bHit && hitResult.bBlockingHit) ? hitResult.Location : startLocation;
	
	// 바닥 체크, WorldStatic, WorldDynamic로 트레이스
	endLocation = GetActorLocation() + GetActorLocation().UpVector * -1000.0f;

	bHit = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), startLocation, endLocation, objectQueryParams
		, true, { this }, bDrawDebugLine ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitResult, true, FLinearColor::Red, FLinearColor::Green, 20.0f
	);

	// 결과 위치를 resultLocation에 저장
	resultLocation = (bHit && hitResult.bBlockingHit) ? hitResult.Location : GetActorLocation();
	
	//offset 적용하여 액터 월드 위치 set
	const float finalZOffset = ZOffsetOverride != -0.1f ? ItemInfo.ItemZOffset : ItemInfo.InitialLocation.Z;
	resultLocation.Z += finalZOffset;
	SetActorLocation(resultLocation, false, nullptr, ETeleportType::TeleportPhysics);
	DrawDebugSphere(GetWorld(), resultLocation, 10.0f, 12, FColor::Green, false, 50.0f);
}

void AItemBase::OnOverlapBegins(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor->ActorHasTag("Player")) return;
	
	//UI Activate
	PlayerRef = Cast<AMainCharacterBase>(OtherActor);
}

void AItemBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || OtherActor->ActorHasTag("Player")) return;

	//UI Deactivate
	PlayerRef = Cast<AMainCharacterBase>(OtherActor);
}

void AItemBase::OnItemPicked_Implementation()
{
	AMainCharacterBase* playerRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!IsValid(playerRef)) return;
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ItemPickEffect, GetActorLocation());

	//제거하면 안된다는 표시를 하는 flag
	bool bNotDestroy = false;
	int32 newItemAmount = ItemInfo.ItemAmount;
	FItemInfoDataStruct currItemInfo = playerRef->ItemInventory->GetSubWeaponInfoData();
	const int32 targetItemID = ItemInfo.ItemID - ITEM_WEAPON_ID_DIFF_VALUE;

	if (IsValid(playerRef->ItemInventory))
	{
		playerRef->ItemInventory->AddItem(ItemInfo.ItemID, ItemInfo.ItemAmount);
	}

	switch (ItemInfo.ItemType)
	{
	case EItemCategoryInfo::E_Ability:
		ItemInfo.ItemAmount = 1;
		break;
	case EItemCategoryInfo::E_Weapon:
		ItemInfo.ItemAmount = 1;
		if (playerRef->ActorHasTag("Player"))
		{
			playerRef->EquipWeapon(targetItemID);
		}
		break;
	case EItemCategoryInfo::E_SubWeapon:
		if (playerRef->ActorHasTag("Player") && currItemInfo.ItemID != ItemInfo.ItemID)
		{
			playerRef->EquipWeapon(targetItemID);

			if (currItemInfo.ItemID != 0 && currItemInfo.ItemType == EItemCategoryInfo::E_SubWeapon)
			{
				UE_LOG(LogTemp, Warning, TEXT("CurrItemInfo -----------------"));
				UE_LOG(LogTemp, Warning, TEXT("CurrItemInfo.ItemID : %d"), currItemInfo.ItemID);
				UE_LOG(LogTemp, Warning, TEXT("CurrItemImage : %s"), *UKismetSystemLibrary::GetDisplayName(currItemInfo.ItemImage));
				UE_LOG(LogTemp, Warning, TEXT("CurrItemInfo.ItemType : %d"), (int32)currItemInfo.ItemType);
				UE_LOG(LogTemp, Warning, TEXT("CurrItemInfo.ItemAmount : %d"), currItemInfo.ItemAmount);
				UE_LOG(LogTemp, Warning, TEXT("NewItemInfo.ItemID : %d"), ItemInfo.ItemID);
				UE_LOG(LogTemp, Warning, TEXT("NewItemInfo.ItemImage : %s"), *UKismetSystemLibrary::GetDisplayName(ItemInfo.ItemImage));
				UE_LOG(LogTemp, Warning, TEXT("NewItemInfo.ItemType : %d"), (int32)ItemInfo.ItemType);
				UE_LOG(LogTemp, Warning, TEXT("NewItemInfo.ItemAmount : %d"), ItemInfo.ItemAmount);

				InitItem(currItemInfo.ItemID, currItemInfo);
				bNotDestroy = true;
			}
		}
		break;
	}	

	if (!bNotDestroy)
	{
		Destroy();
	}
}

void AItemBase::InitializeItemMesh()
{
	if (ItemInfo.ItemID == 0) return;
	if (ItemInfo.ItemMesh.IsNull() || !IsValid(ItemInfo.ItemMesh.Get())) return;
	
	MeshComponent->SetStaticMesh(ItemInfo.ItemMesh.Get());
	MeshComponent->SetRelativeRotation(ItemInfo.InitialRotation);
	MeshComponent->SetRelativeScale3D(ItemInfo.InitialScale);

	ParticleComponent->SetAsset(ItemInfo.ItemEffect.Get());
	ParticleComponent->SetRelativeLocation(ItemInfo.EffectInitialLocation);
	ParticleComponent->SetRelativeRotation(ItemInfo.EffectInitialRotation);
	ParticleComponent->Activate();

	OnItemInitialized.Broadcast(ItemInfo);
	ActivateItem();
	AdjustItemZLocation(ItemInfo.ItemZOffset, true);
}

FItemInfoDataStruct AItemBase::GetItemInfoWithID(const int32 ItemID)
{
	if (ItemDataInfoTable != nullptr && ItemID != 0)
	{
		FItemInfoDataStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(ItemID);

		newInfo = ItemDataInfoTable->FindRow<FItemInfoDataStruct>(FName(rowName), rowName);

		if (newInfo != nullptr) return *newInfo;
	}
	return FItemInfoDataStruct(); 
}

void AItemBase::SetLaunchDirection(FVector NewDirection)
{
	ProjectileMovement->Velocity = NewDirection;
}

void AItemBase::ActivateProjectileMovement()
{
	ProjectileMovement->Activate();
}