// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/RewardBoxBase.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "../../Character/public/MainCharacterBase.h"
#include "../../Character/public/AbilityManagerBase.h"
#include "../../Character/public/CharacterInfoEnum.h"


// Sets default values
ARewardBoxBase::ARewardBoxBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = OverlapVolume = CreateDefaultSubobject<USphereComponent>("SPHERE_COLLISION");
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

	SelectRandomAbilityIdx();
	OverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &ARewardBoxBase::OnPlayerBeginOverlap);
	OverlapVolume->OnComponentEndOverlap.AddDynamic(this, &ARewardBoxBase::OnPlayerEndOverlap);
	OnPlayerBeginInteract.AddDynamic(this, &ARewardBoxBase::AddAbilityToPlayer);

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

void ARewardBoxBase::AddAbilityToPlayer(AMainCharacterBase* PlayerCharacterRef)
{
	if (!IsValid(PlayerCharacterRef)) return;

	const bool result = PlayerCharacterRef->TryAddNewAbility(AbilityType);
	if (result)
	{
		if (DestroyEffectParticle && DestroyEffectSound)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyEffectParticle, GetActorLocation());
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestroyEffectSound, GetActorLocation());
		}
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
	AbilityType = (ECharacterAbilityType)((uint8)FMath::RandRange(1.0f, 7.0f));
}