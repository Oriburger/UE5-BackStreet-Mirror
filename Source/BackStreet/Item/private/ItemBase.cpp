// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/ItemBase.h"
#include "../public/WeaponBase.h"
#include "Components/WidgetComponent.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../../Global/public/DebuffManager.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/ProjectileMovementComponent.h"

#define DEFAULT_ITEM_LAUNCH_SPEED 500.0f
#define ITEM_WEAPON_ID_DIFF_VALUE 20000 
#define ITEM_BULLET_ID_DIFF_VALUE 20001

// Sets default values
AItemBase::AItemBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	this->Tags.Add(FName("Item"));

	PrimaryActorTick.bCanEverTick = false;
	RootComponent = RootCollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("SPHERE_COLLISION_ROOT"));
	RootCollisionVolume->SetCollisionProfileName("Item", true);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_MESH"));
	MeshComponent->SetupAttachment(RootComponent);

	OutlineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_OUTLINE_MESH"));
	OutlineMeshComponent->SetupAttachment(MeshComponent);

	InfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ITEM_INFO_WIDGET"));
	InfoWidgetComponent->SetupAttachment(MeshComponent);
	InfoWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	InfoWidgetComponent->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));

	ParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ITEM_NIAGARA_COMPONENT"));
	ParticleComponent->SetupAttachment(MeshComponent);
	ParticleComponent->bAutoActivate = false;

	ItemTriggerVolume = CreateDefaultSubobject<USphereComponent>("SPHERE_COLLISION");
	ItemTriggerVolume->SetupAttachment(RootComponent);
	ItemTriggerVolume->SetRelativeScale3D(FVector(5.0f));
	ItemTriggerVolume->SetCollisionProfileName("ItemTrigger", true);
	ItemTriggerVolume->OnComponentBeginOverlap.AddUniqueDynamic(this, &AItemBase::OnOverlapBegins);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PROJECTILE_MOVEMENT"));
	ProjectileMovement->InitialSpeed = DEFAULT_ITEM_LAUNCH_SPEED;
	ProjectileMovement->MaxSpeed = DEFAULT_ITEM_LAUNCH_SPEED;
	ProjectileMovement->bAutoActivate = false;	
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	GamemodeRef = Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
	OnPlayerBeginPickUp.BindUFunction(this, FName("OnItemPicked"));

	ActivateItem();
}

void AItemBase::InitItem(int32 NewItemID)
{
	ItemInfo = GetItemInfoWithID(NewItemID);

	if (ItemInfo.ItemID == 0) return;

	if (!ItemInfo.ItemMesh.IsNull())
	{
		TArray<FSoftObjectPath> assetToStream;
		assetToStream.AddUnique(ItemInfo.ItemMesh.ToSoftObjectPath());
		assetToStream.AddUnique(ItemInfo.OutlineMaterial.ToSoftObjectPath());
		if (IsValid(ItemInfo.ItemMesh.Get()))
		{
			InitializeItemMesh();
		}
		else
		{
			FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
			streamable.RequestAsyncLoad(assetToStream, FStreamableDelegate::CreateUObject(this, &AItemBase::InitializeItemMesh));
		}
	}
	//ItemInfo.ItemName
	//ItemInfo.OutlineColor
}

void AItemBase::OnOverlapBegins(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor->ActorHasTag("Player")) return;
	
	//UI Activate
}

void AItemBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || OtherActor->ActorHasTag("Player")) return;

	//UI Deactivate
}

void AItemBase::OnItemPicked(AActor* Causer)
{
	if (!IsValid(Causer) || Causer->IsActorBeingDestroyed()) return;

	AMainCharacterBase* playerRef = Cast<AMainCharacterBase>(Causer);
	AWeaponInventoryBase* playerInventoryRef = playerRef->GetInventoryRef();
	check(IsValid(playerInventoryRef)); //player가 살아있는데, Inventory가 Invalid하면 안됨

	switch (ItemInfo.ItemType)
	{
	case EItemCategoryInfo::E_Weapon:
		if (Causer->ActorHasTag("Player"))
		{
			const int32 targetWeaponID = ItemInfo.ItemID - ITEM_WEAPON_ID_DIFF_VALUE;
			UE_LOG(LogTemp, Warning, TEXT("targetWeaopnID : %d"), targetWeaponID);
			if (!playerRef->PickWeapon(targetWeaponID)) return;
		}
		break;
	case EItemCategoryInfo::E_Bullet:
		if (Causer->ActorHasTag("Player"))
		{
			const int32 targetWeaponID = ItemInfo.ItemID - ITEM_BULLET_ID_DIFF_VALUE;
			//FWeaponStatStruct targetWeaponStat = playerInventoryRef->GetWeaponStatInfoWithID(targetWeaponID);

			if (playerInventoryRef->GetWeaponIsContained(targetWeaponID))
			{
			//	ensure(playerInventoryRef->TryAddAmmoToWeapon(targetWeaponID, (int32)ItemInfo.Variable));
			}
			else
			{
				//if (targetWeaponStat.WeaponType == EWeaponType::E_Throw)
				{
				//	ensure(playerRef->PickWeapon(targetWeaponID));
				}
				//else
				{
					GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName("탄환에 맞는 무기를 소지해주세요"), FColor(255));
				}
			}
		}
		break;
	case EItemCategoryInfo::E_StatUp:
		break;
	case EItemCategoryInfo::E_Mission:
		break;
	}
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ItemPickEffect, GetActorLocation());
	Destroy();
}

void AItemBase::InitializeItemMesh()
{
	if (ItemInfo.ItemMesh.IsNull() || !IsValid(ItemInfo.ItemMesh.Get())) return;
	
	MeshComponent->SetStaticMesh(ItemInfo.ItemMesh.Get());
	MeshComponent->SetRelativeLocation(ItemInfo.InitialLocation);
	MeshComponent->SetWorldRotation(ItemInfo.InitialRotation);
	MeshComponent->SetRelativeScale3D(ItemInfo.InitialScale);

	OutlineMeshComponent->SetStaticMesh(ItemInfo.ItemMesh.Get());
	OutlineMeshComponent->SetRelativeLocation(FVector(0.0f));
	OutlineMeshComponent->SetRelativeRotation(FRotator(0.0f));
	OutlineMeshComponent->SetRelativeScale3D(FVector(1.0f));
	
	for (int32 matIdx = 0; matIdx < OutlineMeshComponent->GetNumMaterials(); matIdx++)
	{
		if(!IsValid(ItemInfo.OutlineMaterial.Get())) break;
		OutlineMeshComponent->SetMaterial(matIdx, ItemInfo.OutlineMaterial.Get());
	}
}

FItemInfoStruct AItemBase::GetItemInfoWithID(const int32 ItemID)
{
	if (ItemDataInfoTable != nullptr && ItemID != 0)
	{
		FItemInfoStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(ItemID);

		newInfo = ItemDataInfoTable->FindRow<FItemInfoStruct>(FName(rowName), rowName);

		if (newInfo != nullptr) return *newInfo;
	}
	return FItemInfoStruct(); 
}

void AItemBase::SetLaunchDirection(FVector NewDirection)
{
	ProjectileMovement->Velocity = NewDirection;
}

void AItemBase::ActivateProjectileMovement()
{
	ProjectileMovement->Activate();
}