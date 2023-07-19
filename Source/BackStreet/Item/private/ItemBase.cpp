// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/ItemBase.h"
#include "../public/WeaponBase.h"
#include "Components/WidgetComponent.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
#include "../../Character/public/CharacterBase.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../../Global/public/DebuffManager.h"
#include "GameFramework/ProjectileMovementComponent.h"

#define DEFAULT_ITEM_LAUNCH_SPEED 500.0f

// Sets default values
AItemBase::AItemBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	this->Tags.Add(FName("Item"));

	PrimaryActorTick.bCanEverTick = true;
	RootComponent = RootCollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("SPHERE_COLLISION_ROOT"));
	RootCollisionVolume->SetCollisionProfileName("Item", true);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_MESH"));
	MeshComponent->SetupAttachment(RootComponent);

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
}

void AItemBase::InitItem(EItemCategoryInfo SetType, uint8 NewItemID)
{
	ItemType = SetType;
	ItemID = NewItemID;
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

void AItemBase::SetLaunchDirection(FVector NewDirection)
{
	ProjectileMovement->Velocity = NewDirection;
}

void AItemBase::ActivateProjectileMovement()
{
	ProjectileMovement->Activate();
}