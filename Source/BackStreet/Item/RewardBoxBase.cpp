// Fill out your copyright notice in the Description page of Project Settings.


#include "RewardBoxBase.h"
#include "../System/AbilitySystem/AbilityManagerBase.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "../Character/Struct/CharacterInfoEnum.h"
#include "../Item/InteractiveCollisionComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"


// Sets default values
ARewardBoxBase::ARewardBoxBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = OverlapVolume = CreateDefaultSubobject<UInteractiveCollisionComponent>("SPHERE_COLLISION");
	OverlapVolume->SetRelativeScale3D(FVector(5.0f));
	OverlapVolume->SetCollisionProfileName("ItemTrigger", true);
	OverlapVolume->SetSimulatePhysics(true);
	OverlapVolume->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ITEM_MESH"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionProfileName("Item", false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	InfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("INFO_WIDGET"));
	InfoWidgetComponent->SetupAttachment(Mesh);
	InfoWidgetComponent->SetVisibility(false);

	this->Tags.Add("RewardBox");
}

// Called when the game starts or when spawned
void ARewardBoxBase::BeginPlay()
{
	Super::BeginPlay();

	//에디터에 지정해주지 않았다면, 
	//if (AbilityType == ECharacterAbilityType::E_None) SelectRandomAbilityIdx();
	OverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &ARewardBoxBase::OnPlayerBeginOverlap);
	OverlapVolume->OnComponentEndOverlap.AddDynamic(this, &ARewardBoxBase::OnPlayerEndOverlap);
	OverlapVolume->OnInteractionBegin.AddDynamic(this, &ARewardBoxBase::AddAbilityToPlayer);

	InitializeWidgetComponent();
}

// Called every frame
void ARewardBoxBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARewardBoxBase::OnPlayerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !OtherActor->ActorHasTag("Player")) return;

	ActivateWidgetComponent(false);
}

void ARewardBoxBase::OnPlayerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || !OtherActor->ActorHasTag("Player")) return;

	ActivateWidgetComponent(true);
}

void ARewardBoxBase::AddAbilityToPlayer()
{
	AMainCharacterBase* playerRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!IsValid(playerRef)) return;

	const bool result = playerRef->TryAddNewAbility(AbilityID);
	if (result)
	{
		if (DestroyEffectParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyEffectParticle, GetActorLocation());
		}
		if (OnAddAbility.IsBound())
			OnAddAbility.Broadcast();
		Destroy();
	}
	else
	{
		//GetWorld()->GetAuthGameMode<ABackStreetGamemodeBase>()->
		//시스템 메시지
	}
}

void ARewardBoxBase::SelectRandomAbilityIdx()
{
	//임시코드!!!!!!! 반드시 변경 필요 - @ljh
	UE_LOG(LogTemp, Log, TEXT("Call ARewardBoxBase::SelectRandomAbilityIdx()"));
	AbilityID = ((int32)FMath::RandRange(1.0f, 16.0f));
}