// Fill out your copyright notice in the Description page of Project Settings.
#include "ItemBase.h"
#include "../Global/BackStreetGameModeBase.h"
#include "../Character/CharacterBase.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
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
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	DefaultSceneRoot->SetupAttachment(RootComponent);
	SetRootComponent(DefaultSceneRoot);
		
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_STATIC_MESH"));
	MeshComponent->SetupAttachment(DefaultSceneRoot);
	MeshComponent->SetCollisionProfileName("Item", true);

	OutlineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_OUTLINE_MESH"));
	OutlineMeshComponent->SetupAttachment(MeshComponent);

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
	ItemTriggerVolume->OnInteractionBegin.AddDynamic(this, &AItemBase::OnItemPicked);
	PlayerRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	ActivateItem();
}

void AItemBase::InitItem(int32 NewItemID, FItemInfoDataStruct InfoOverride)
{
	ItemInfo = InfoOverride.ItemID == 0 ? GetItemInfoWithID(NewItemID) : InfoOverride;

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
	OnItemInitialized.Broadcast(ItemInfo);
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
	
	switch (ItemInfo.ItemType)
	{
	case EItemCategoryInfo::E_Weapon:
		if (playerRef->ActorHasTag("Player"))
		{
			const int32 targetWeaponID = ItemInfo.ItemID - ITEM_WEAPON_ID_DIFF_VALUE;
			playerRef->EquipWeapon(targetWeaponID);
		}
		break;
	case EItemCategoryInfo::E_SubWeapon:
		if (playerRef->ActorHasTag("Player"))
		{
			const int32 targetWeaponID = ItemInfo.ItemID - ITEM_WEAPON_ID_DIFF_VALUE;
			playerRef->EquipWeapon(targetWeaponID);
		}
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
	MeshComponent->SetRelativeRotation(ItemInfo.InitialRotation);
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
	OnItemInitialized.Broadcast(ItemInfo);
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