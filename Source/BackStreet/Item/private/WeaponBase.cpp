// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/WeaponBase.h"
#include "Components/AudioComponent.h"
#include "../../Global/public/DebuffManager.h"
#include "../../Character/public/CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Global/public/BackStreetGameModeBase.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	DefaultSceneRoot->SetupAttachment(RootComponent);
	SetRootComponent(DefaultSceneRoot);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WEAPON_MESH"));
	WeaponMesh->SetupAttachment(DefaultSceneRoot);

	MeleeTrailParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ITEM_NIAGARA_COMPONENT"));
	MeleeTrailParticle->SetupAttachment(WeaponMesh);
	MeleeTrailParticle->bAutoActivate = false;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	InitWeapon();
}

void AWeaponBase::UpdateWeaponStat(FWeaponStatStruct NewStat)
{
}

void AWeaponBase::InitWeapon()
{
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	//무기 스탯이 초기화가 되지 않았다면
	if (WeaponStat.WeaponName == FName(""))
	{
		GamemodeRef->UpdateWeaponStatWithID(this, WeaponID);
	}
	WeaponState.CurrentDurability = WeaponStat.MaxDurability;
}

void AWeaponBase::RevertWeaponInfo(FWeaponStatStruct OldWeaponStat, FWeaponStateStruct OldWeaponState)
{
	if (OldWeaponStat.WeaponName == FName("")) return;
	WeaponStat = OldWeaponStat;
	WeaponState = OldWeaponState;
}

void AWeaponBase::Attack() {}

void AWeaponBase::StopAttack() { }

void AWeaponBase::UpdateComboState()
{
	WeaponState.ComboCount = (WeaponState.ComboCount + 1); 
}

void AWeaponBase::SetResetComboTimer()
{
	GetWorldTimerManager().ClearTimer(ComboTimerHandle);
	const float delayValue = FMath::Clamp((-1.0f * WeaponStat.WeaponAtkSpeedRate) + 2.0f, 0.25f, 10.0f);

	GetWorldTimerManager().SetTimer(ComboTimerHandle, FTimerDelegate::CreateLambda([&]() {
		WeaponState.ComboCount = 0;
	}), delayValue, false);
}

void AWeaponBase::SetOwnerCharacter(ACharacterBase* NewOwnerCharacterRef)
{
	if (!IsValid(NewOwnerCharacterRef)) return;
	OwnerCharacterRef = NewOwnerCharacterRef;
	SetOwner(OwnerCharacterRef.Get());

	//Melee Trail Particle 파라미터 초기화
	if (IsValid(MeleeTrailParticle))
	{
		FVector startLocation = WeaponMesh->GetSocketLocation("Mid");
		FVector endLocation = WeaponMesh->GetSocketLocation("End");

		MeleeTrailParticle->SetVectorParameter(FName("Start"), startLocation);
		MeleeTrailParticle->SetVectorParameter(FName("End"), endLocation);
		MeleeTrailParticle->SetColorParameter(FName("Color"), MeleeTrailParticleColor);
	}
}

void AWeaponBase::PlayEffectSound(USoundCue* EffectSound)
{
	if (EffectSound == nullptr || !OwnerCharacterRef.IsValid() || !OwnerCharacterRef.Get()->ActorHasTag("Player")) return;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), EffectSound, GetActorLocation());
}

void AWeaponBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(ComboTimerHandle);
}

void AWeaponBase::UpdateDurabilityState()
{
	if (OwnerCharacterRef.Get()->GetCharacterStat().bInfinite || WeaponStat.bInfinite) return;
	if (--WeaponState.CurrentDurability == 0)
	{
		if (IsValid(DestroyEffectParticle))
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyEffectParticle, GetActorLocation(), FRotator()); //파괴 효과
		}
		ClearAllTimerHandle();
		OwnerCharacterRef.Get()->StopAttack();
		OnWeaponBeginDestroy.Broadcast();
	}
}

float AWeaponBase::GetAttackRange() 
{ 
	if(WeaponStat.WeaponType == EWeaponType::E_Melee) return 25.0f;
	return 200.0f;
}