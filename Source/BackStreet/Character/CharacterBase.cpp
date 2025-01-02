#include "CharacterBase.h"
#include "./Component/DebuffManagerComponent.h"
#include "./Component/TargetingManagerComponent.h"
#include "./Component/WeaponComponentBase.h"
#include "./Component/SkillManagerComponentBase.h"
#include "./Component/ActionTrackingComponent.h"
#include "./EnemyCharacter/EnemyCharacterBase.h"
#include "../Item/Weapon/Ranged/RangedCombatManager.h"
#include "../Global/BackStreetGameModeBase.h"
#include "../System/AssetSystem/AssetManagerBase.h"
#include "../System/SkillSystem/SkillBase.h"
#include "../System/AISystem/AIControllerBase.h"
#include "Engine/DamageEvents.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->Tags.Add("Character");

	HitSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HIT_SCENE"));
	HitSceneComponent->SetupAttachment(GetMesh());
	HitSceneComponent->SetRelativeLocation(FVector(0.0f, 115.0f, 90.0f));

	DebuffManagerComponent = CreateDefaultSubobject<UDebuffManagerComponent>(TEXT("DEBUFF_MANAGER"));
	TargetingManagerComponent = CreateDefaultSubobject<UTargetingManagerComponent>(TEXT("TARGETING_MANAGER"));;
	ActionTrackingComponent = CreateDefaultSubobject<UActionTrackingComponent>(TEXT("ACTION_TRACKER"));
	SkillManagerComponent = CreateDefaultSubobject<USkillManagerComponentBase>(TEXT("SKILL_MANAGER"));

	WeaponComponent = CreateDefaultSubobject<UWeaponComponentBase>(TEXT("WeaponBase"));
	WeaponComponent->SetupAttachment(GetMesh(), FName("Weapon_R"));

	BuffNiagaraEmitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BUFF_EFFECT"));
	BuffNiagaraEmitter->SetupAttachment(GetMesh());
	BuffNiagaraEmitter->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	BuffNiagaraEmitter->bAutoActivate = false;

	DirectionNiagaraEmitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DIRECTION_EFFECT"));
	DirectionNiagaraEmitter->SetupAttachment(GetMesh());
	DirectionNiagaraEmitter->SetRelativeRotation({ 0.0f, 90.0f, 0.0f });

	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
	InitializeActionTriggerDelegateMap();
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CharacterID = AssetSoftPtrInfo.CharacterID;

	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GamemodeRef.IsValid())
	{
		AssetManagerBaseRef = GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();
	}
	LandedDelegate.AddDynamic(this, &ACharacterBase::OnPlayerLanded);
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacterBase::InitializeActionTriggerDelegateMap()
{ 
	//Init List, 액션 및 델리게이트 추가 시 필수
	ActionTriggerDelegateMap.Empty();
	ActionTriggerDelegateMap.Add("OnMoveStarted", &OnMoveStarted);
	ActionTriggerDelegateMap.Add("OnJumpStarted", &OnJumpStarted);
	ActionTriggerDelegateMap.Add("OnJumpEnd", &OnJumpEnd);
	ActionTriggerDelegateMap.Add("OnRollStarted", &OnRollStarted);
	ActionTriggerDelegateMap.Add("OnSprintStarted", &OnSprintStarted);
	ActionTriggerDelegateMap.Add("OnSprintEnd", &OnSprintEnd);
	ActionTriggerDelegateMap.Add("OnRollEnded", &OnRollEnded);
	ActionTriggerDelegateMap.Add("OnAttackStarted", &OnAttackStarted);
	ActionTriggerDelegateMap.Add("OnJumpAttackStarted", &OnJumpAttackStarted);
	ActionTriggerDelegateMap.Add("OnDashAttackStarted", &OnDashAttackStarted);
	ActionTriggerDelegateMap.Add("OnDamageReceived", &OnDamageReceived);
	ActionTriggerDelegateMap.Add("OnSkillStarted", &OnSkillStarted);
	ActionTriggerDelegateMap.Add("OnSkillEnded", &OnSkillEnded);
	ActionTriggerDelegateMap.Add("OnDeath", &OnDeath);
}

void ACharacterBase::ResetAtkIntervalTimer()
{
	CharacterGameplayInfo.bCanAttack = true;
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
}

void ACharacterBase::SetLocationWithInterp(FVector NewValue, float InterpSpeed, const bool bAutoReset)
{
	FTimerDelegate updateFunctionDelegate;

	//Binding the function with specific values
	OnBeginLocationInterp.Broadcast();
	updateFunctionDelegate.BindUFunction(this, FName("UpdateLocation"), NewValue, InterpSpeed, bAutoReset);

	//Calling MyUsefulFunction after 5 seconds without looping
	ResetLocationInterpTimer();
	if (FVector::Distance(GetActorLocation(), NewValue) > 150.0f) //벽뚫기 방지코드
	{
		GetWorld()->GetTimerManager().SetTimer(LocationInterpHandle, updateFunctionDelegate, 0.01f, true);
	}
}

void ACharacterBase::ResetLocationInterpTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);
}

void ACharacterBase::PlayHitAnimMontage()
{
	if (GetIsActionActive(ECharacterActionType::E_Skill)) return;

	AAIControllerBase* aiControllerRef = nullptr;
	aiControllerRef = Cast<AAIControllerBase>(Controller);

	if (!IsValid(aiControllerRef) && aiControllerRef->GetBehaviorState() != EAIBehaviorType::E_Skill) return;

	const int32 randomIdx = UKismetMathLibrary::RandomIntegerInRange(0, AssetSoftPtrInfo.HitAnimMontageSoftPtrList.Num() - 1);
	if (AssetHardPtrInfo.HitAnimMontageList.Num() > 0 && IsValid(AssetHardPtrInfo.HitAnimMontageList[randomIdx]))
	{
		PlayAnimMontage(AssetHardPtrInfo.HitAnimMontageList[randomIdx]);
	}
}

void ACharacterBase::PlayKnockBackAnimMontage()
{
	if (GetIsActionActive(ECharacterActionType::E_Skill)) return;

	AAIControllerBase* aiControllerRef = nullptr;
	aiControllerRef = Cast<AAIControllerBase>(Controller);

	if (!IsValid(aiControllerRef) && aiControllerRef->GetBehaviorState() != EAIBehaviorType::E_Skill) return;

	if (AssetHardPtrInfo.KnockdownAnimMontageList.Num() > 0 && IsValid(AssetHardPtrInfo.KnockdownAnimMontageList[0]))
	{
		PlayAnimMontage(AssetHardPtrInfo.KnockdownAnimMontageList[0]);
	}
}

TArray<FName> ACharacterBase::GetCurrentMontageSlotName()
{
	TArray<FName> slotName;

	UAnimMontage* currentMontage = GetMesh()->GetAnimInstance()->GetCurrentActiveMontage();

	if (IsValid(currentMontage))
	{
		for (int32 slotTrackIndex = 0; slotTrackIndex < currentMontage->SlotAnimTracks.Num(); slotTrackIndex++)
		{
			FSlotAnimationTrack slotAnimationTrack = currentMontage->SlotAnimTracks[slotTrackIndex];
			slotName.Add(slotAnimationTrack.SlotName);
		}
	}

	return slotName;
}

void ACharacterBase::SetAirAtkLocationUpdateTimer()
{
	GetWorldTimerManager().SetTimer(AirAtkLocationUpdateHandle, this, &ACharacterBase::SetAirAttackLocation, 0.1, true);
}

void ACharacterBase::ResetAirAtkLocationUpdateTimer()
{
	CharacterGameplayInfo.bIsAirAttacking = false;
	GetWorldTimerManager().ClearTimer(AirAtkLocationUpdateHandle);
	AirAtkLocationUpdateHandle.Invalidate();
}

void ACharacterBase::UpdateLocation(const FVector TargetValue, const float InterpSpeed, const bool bAutoReset)
{
	FVector currentLocation = GetActorLocation();
	if (currentLocation.Equals(TargetValue, GetCharacterMovement()->IsFalling() ? 100.0f : 25.0f))
	{
		GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);
		OnEndLocationInterp.Broadcast();
		if (bAutoReset)
		{
			//SetLocationWithInterp(GetActorLocation() , InterpSpeed * 1.5f, false);
		}
	}
	currentLocation = FMath::VInterpTo(currentLocation, TargetValue, 0.1f, InterpSpeed);
	SetActorLocation(currentLocation, false, nullptr, ETeleportType::None);
}

void ACharacterBase::SetAirAttackLocation()
{
	if (!GetCharacterMovement()->IsFalling()) return;

	//Set collision query params
	FCollisionQueryParams collisionQueryParam;
	collisionQueryParam.AddIgnoredActor(this);

	//execute linetrace
	TArray<FHitResult> hitResults;
	const float jumpHeight = 800.0f;

	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	TEnumAsByte<EObjectTypeQuery> worldStatic = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic);
	TEnumAsByte<EObjectTypeQuery> worldDynamic = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic);
	objectTypes.Add(worldStatic);
	objectTypes.Add(worldDynamic);

	UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), GetActorLocation()
		, GetActorLocation() - FVector(0.0f, 0.0f, jumpHeight), objectTypes, false, { this }
		, EDrawDebugTrace::None, hitResults, true);

	//For Debug // Do not erase this comment
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() - FVector(0.0f, 0.0f, jumpHeight), FColor::Yellow
	//	, false, 800.0f, 1u, 5.0f);

	if (hitResults.Num() > 0)
	{
		FHitResult result = hitResults[hitResults.Num() - 1];
		if (result.bBlockingHit)
		{
			float dist = FVector::Distance(result.Location, GetActorLocation());
			LaunchCharacter({ 0.0f, 0.0f, (jumpHeight - dist) * 5.0f }, true, true);
		}
	}
}

void ACharacterBase::OnPlayerLanded(const FHitResult& Hit)
{
	CharacterGameplayInfo.bIsAirAttacking = false;
	CharacterGameplayInfo.bIsDownwardAttacking = false;
	OnJumpEnd.Broadcast();

	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Skill)
	{
		//ResetActionState();
	}

	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Die)
	{
		Die();
	}

	//damager side 
	if (this->GetVelocity().Length() >= 3500.0f)
	{
		GamemodeRef.Get()->ActivateSlowHitEffect(0.5f);
		GamemodeRef.Get()->PlayCameraShakeEffect(ECameraShakeType::E_Explosion, GetActorLocation(), 1000.0f);

		//play smash sound
		if (AssetManagerBaseRef.IsValid())
		{
			AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 1, "Chop");
		}
	}
	//Test code for knockdown on ground event
	//UE_LOG(LogTemp, Warning, TEXT("$Land %s  / Speed %.2lf$"), *(this->GetName()), this->GetVelocity().Length());
}

void ACharacterBase::ResetHitCounter()
{
	CharacterGameplayInfo.HitCounter = 0;
}

TArray<UAnimMontage*> ACharacterBase::GetTargetMeleeAnimMontageList()
{
	int32 nextAnimIdx = 0;
	TArray<UAnimMontage*> targetAnimList;

	switch (WeaponComponent->GetWeaponStat().WeaponType)
	{
	case EWeaponType::E_Melee:
		if (!ActorHasTag("Enemy"))
		{
			if (CharacterGameplayInfo.bIsAirAttacking && GetCharacterMovement()->IsFalling())
			{
				if (AssetHardPtrInfo.AirAttackAnimMontageList.Num() > 0)
				{
					nextAnimIdx = ActionTrackingComponent->CurrentComboCount % AssetHardPtrInfo.AirAttackAnimMontageList.Num();
					targetAnimList = AssetHardPtrInfo.AirAttackAnimMontageList;

					if (ActionTrackingComponent->CurrentComboCount == AssetHardPtrInfo.AirAttackAnimMontageList.Num())
					{
						TryDownwardAttack();
						return {};
					}
				}
			}
			else if (CharacterGameplayInfo.bIsAirAttacking || GetCharacterMovement()->IsFalling())
			{
				ResetAirAtkLocationUpdateTimer();
				if (IsValid(TargetingManagerComponent) && IsValid(TargetingManagerComponent->GetTargetedCharacter()))
				{
					TargetingManagerComponent->GetTargetedCharacter()->ResetAirAtkLocationUpdateTimer();
				}
				TryDownwardAttack();
				return {};
			}
		}
		//check melee anim type
		if (AssetHardPtrInfo.NormalComboAnimMontageList.Num() > 0)
		{
			nextAnimIdx = ActionTrackingComponent->CurrentComboCount % AssetHardPtrInfo.NormalComboAnimMontageList.Num();
			targetAnimList = AssetHardPtrInfo.NormalComboAnimMontageList;
		}
		break;

	case EWeaponType::E_Shoot:
		if (AssetHardPtrInfo.ShootAnimMontageList.Num() > 0)
		{
			nextAnimIdx = ActionTrackingComponent->CurrentComboCount % AssetHardPtrInfo.ShootAnimMontageList.Num();
		}
		targetAnimList = AssetHardPtrInfo.ShootAnimMontageList;
		break;
	}
	return targetAnimList;
}

void ACharacterBase::KnockDown()
{
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Die) return;
	ResetHitCounter();

	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_KnockedDown;

	GetWorldTimerManager().ClearTimer(KnockDownDelayTimerHandle);
	KnockDownDelayTimerHandle.Invalidate();

	FTimerDelegate KnockDownDelegate;
	KnockDownDelegate.BindUFunction(this, "StandUp");
	GetWorldTimerManager().SetTimer(KnockDownAnimMontageHandle, KnockDownDelegate, 1.0f, false, 4.5f);
}

void ACharacterBase::StandUp()
{
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Die) return;
	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Idle;
	GetWorldTimerManager().ClearTimer(KnockDownAnimMontageHandle);
}

void ACharacterBase::InitCharacterGameplayInfo(FCharacterGameplayInfo NewGameplayInfo)
{
	if (CharacterGameplayInfo.bUseDefaultStat)
	{
		if (!NewGameplayInfo.IsValid()) return;
		CharacterGameplayInfo = NewGameplayInfo;
		CharacterID = NewGameplayInfo.CharacterID;
	}
	CharacterGameplayInfo.UpdateTotalValues();
	CharacterGameplayInfo.bCanAttack = true;
	CharacterGameplayInfo.bCanRoll = true;
	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Idle;
	CharacterGameplayInfo.CurrentHP = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MaxHealth);
	GetCharacterMovement()->MaxWalkSpeed = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed);
}

float ACharacterBase::GetCurrentHP()
{
	return CharacterGameplayInfo.CurrentHP;
}

float ACharacterBase::GetMaxHP()
{
	return CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MaxHealth);
}

bool ACharacterBase::TryAddNewDebuff(FDebuffInfoStruct DebuffInfo, AActor* Causer)
{
	if (!GamemodeRef.IsValid()) return false;

	bool result = DebuffManagerComponent->SetDebuffTimer(DebuffInfo, Causer);
	return result;
}

bool ACharacterBase::GetDebuffIsActive(ECharacterDebuffType DebuffType)
{
	if (!GamemodeRef.IsValid()) return false;
	return DebuffManagerComponent->GetDebuffIsActive(DebuffType);
}

void ACharacterBase::UpdateWeaponStat(FWeaponStatStruct NewStat)
{
	WeaponComponent->WeaponStat = NewStat;
}

void ACharacterBase::ResetActionState(bool bForceReset)
{
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Die) return;
	if (!bForceReset && CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Stun) return;	

	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Idle;

	//Reset Location Interp Timer Handle
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);

	FWeaponStatStruct currWeaponStat = this->WeaponComponent->GetWeaponStat();
	this->WeaponComponent->SetWeaponStat(currWeaponStat);
	StopAttack();
	 
	if (!GetWorldTimerManager().IsTimerActive(AtkIntervalHandle))
	{
		CharacterGameplayInfo.bCanAttack = true;
	}
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsValid(DamageCauser) || CharacterGameplayInfo.bIsInvincibility || DamageAmount <= 0.0f) return 0.0f;
	//if (GetIsActionActive(ECharacterActionType::E_Die)) return 0.0f;

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	const float& totalHealthValue = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MaxHealth);
	float oldClampedHealthValue = CharacterGameplayInfo.CurrentHP;
	float newClampedHealthValue = 0.0f;

	CharacterGameplayInfo.CurrentHP = CharacterGameplayInfo.CurrentHP - DamageAmount * (1.0f - FMath::Clamp(0.5f * CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_Defense), 0.0f, 0.5f));
	newClampedHealthValue = CharacterGameplayInfo.CurrentHP = FMath::Max(0.0f, CharacterGameplayInfo.CurrentHP);
	
	oldClampedHealthValue = UKismetMathLibrary::MapRangeClamped(oldClampedHealthValue, 0.0f, totalHealthValue, 0.0f, 100.0f);
	newClampedHealthValue = UKismetMathLibrary::MapRangeClamped(newClampedHealthValue, 0.0f, totalHealthValue, 0.0f, 100.0f);
	
	OnHealthChanged.Broadcast(oldClampedHealthValue, newClampedHealthValue);
	OnDamageReceived.Broadcast();

	// =========== 사망 및 애니메이션 처리 ==============
	if (CharacterGameplayInfo.CurrentHP == 0.0f)
	{
		CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Die;
		if (!GetCharacterMovement()->IsFalling())
		{
			Die();
		}
	}

	//if causer is not character(like resource manager), return func immediately
	if (!DamageCauser->ActorHasTag("Character")) return 0.0f;

	// ====== move to enemy's Hit scene location ======================
	//Update EnemyCharacter's Rotation
	FVector location = HitSceneComponent->GetComponentLocation();
	FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(DamageCauser->GetActorLocation(), GetActorLocation());
	newRotation.Pitch = newRotation.Roll = 0.0f;

	if (DamageCauser->ActorHasTag("Player") && DamageCauser != this)
	{
		ACharacterBase* causerTarget = Cast<ACharacterBase>(DamageCauser)->TargetingManagerComponent->GetTargetedCharacter();
		if (IsValid(causerTarget) && causerTarget == this)
		{
			Cast<ACharacterBase>(DamageCauser)->SetActorRotation(newRotation);
			newRotation.Yaw += 180.0f;
			if (!ActorHasTag("Boss"))
			{
				SetActorRotation(newRotation);
			}
		}
	}

	//Reset Location Interp Timer Handle
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);

	// ====== Hit Counter & Knock Down Check ===========================
	CharacterGameplayInfo.HitCounter += 1;

	// Set hit counter reset event using retriggable timer
	GetWorldTimerManager().ClearTimer(HitCounterResetTimerHandle);
	HitCounterResetTimerHandle.Invalidate();
	GetWorldTimerManager().SetTimer(HitCounterResetTimerHandle, this, &ACharacterBase::ResetHitCounter, 1.0f, false, 1.0f);

	// Check knock down condition and set knock down event using retriggable timer
	//if (Cast<ACharacterBase>(DamageCauser)->WeaponComponent->GetWeaponStat().FinalImpactStrength > 0 && Cast<ACharacterBase>(DamageCauser)->WeaponComponent->GetIsFinalCombo())
	{
		//KnockDown();
		/*
		GetWorldTimerManager().ClearTimer(KnockDownDelayTimerHandle);
		KnockDownDelayTimerHandle.Invalidate();
		GetWorldTimerManager().SetTimer(KnockDownDelayTimerHandle, this, &ACharacterBase::KnockDown, 1.0f, false, 0.1f);
		*/
	}
	return DamageAmount;
}

float ACharacterBase::TakeDebuffDamage(ECharacterDebuffType DebuffType, float DamageAmount, AActor* Causer)
{
	if (!IsValid(Causer) || Causer->IsActorBeingDestroyed()) return 0.0f;
	if (GetIsActionActive(ECharacterActionType::E_Die)) return 0.0f;
	float totalDamage = DamageAmount;
	totalDamage = FMath::Min(totalDamage, DamageAmount);
	totalDamage = FMath::Max(0.0f, totalDamage);

	TakeDamage(DamageAmount, FDamageEvent(), nullptr, this);
	return DamageAmount;
}

void ACharacterBase::SetActionState(ECharacterActionType Type)
{
	CharacterGameplayInfo.CharacterActionState = Type;
	return;
}

void ACharacterBase::TakeHeal(float HealAmount, bool bIsTimerEvent, uint8 BuffDebuffType)
{
	const float& totalHealthValue = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MaxHealth);
	float oldClampedHealthValue = CharacterGameplayInfo.CurrentHP;
	float newClampedHealthValue = 0.0f;
	CharacterGameplayInfo.CurrentHP += (HealAmount * (1.0f + GetStatTotalValue(ECharacterStatType::E_HealItemPerformance)));
	newClampedHealthValue = CharacterGameplayInfo.CurrentHP = FMath::Min(totalHealthValue, CharacterGameplayInfo.CurrentHP);
	OnDamageReceived.Broadcast();
	
	oldClampedHealthValue = UKismetMathLibrary::MapRangeClamped(oldClampedHealthValue, 0.0f, totalHealthValue, 0.0f, 100.0f);
	newClampedHealthValue = UKismetMathLibrary::MapRangeClamped(newClampedHealthValue, 0.0f, totalHealthValue, 0.0f, 100.0f);

	OnHealthChanged.Broadcast(oldClampedHealthValue, newClampedHealthValue);

	return;
}

void ACharacterBase::ApplyKnockBack(AActor* Target, float Strength)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return;
	if (Cast<ACharacterBase>(Target)->GetCharacterMovement()->IsFalling()) return;
	if (GetIsActionActive(ECharacterActionType::E_Skill)) return;
	if (GetIsActionActive(ECharacterActionType::E_Die)) return;

	FVector knockBackDirection = Target->GetActorLocation() - GetActorLocation();
	knockBackDirection = knockBackDirection.GetSafeNormal();
	knockBackDirection *= Strength;
	knockBackDirection.Z = 1.0f;

	if (Target->ActorHasTag("Enemy"))
	{
		AEnemyCharacterBase* knockBackCharacter = Cast<AEnemyCharacterBase>(Target);
		knockBackCharacter->TakeKnockBack(Strength, DefaultStat.DefaultKnockBackResist);
	}
}

void ACharacterBase::Die()
{
	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	//모든 타이머를 제거한다. (타이머 매니저의 것도)
	ClearAllTimerHandle();

	//무적 처리를 하고, Movement를 비활성화
	CharacterGameplayInfo.bIsInvincibility = true;
	GetCharacterMovement()->Deactivate();
	bUseControllerRotationYaw = false;

	OnDeath.Broadcast();

	if (AssetHardPtrInfo.DieAnimMontageList.Num() > 0
		&& IsValid(AssetHardPtrInfo.DieAnimMontageList[0]))
	{
		PlayAnimMontage(AssetHardPtrInfo.DieAnimMontageList[0]);
	}
	else
	{
		Destroy();
	}
}

void ACharacterBase::TryAttack()
{
	if (GetWorldTimerManager().IsTimerActive(AtkIntervalHandle)) return;
	if (!CharacterGameplayInfo.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return;

	int32 nextAnimIdx = ActionTrackingComponent->CurrentComboCount;
	const float attackSpeed = FMath::Clamp(GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_NormalAttackSpeed) * WeaponComponent->GetWeaponStat().WeaponAtkSpeedRate, 0.2f, 1.5f);

	//Choose animation which fit battle situation
	TArray <UAnimMontage*> targetAnimList = GetTargetMeleeAnimMontageList();
	if (targetAnimList.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("ACharacterBase::TryAttack() - ID : %d, Combo type %d, anim montage list is empty"), CharacterID, (int32)ActionTrackingComponent->CurrentComboType);
		return;
	}
	nextAnimIdx %= targetAnimList.Num();
	if (targetAnimList.IsValidIndex(nextAnimIdx)
		&& IsValid(targetAnimList[nextAnimIdx]))
	{
		PlayAnimMontage(targetAnimList[nextAnimIdx], attackSpeed + 0.25f);
		CharacterGameplayInfo.bCanAttack = false; //공격간 Delay,Interval 조절을 위해 세팅
		CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Attack;
		OnAttackStarted.Broadcast();
	}
}

void ACharacterBase::TryUpperAttack()
{/*
	if (GetCharacterMovement()->IsFalling()) return;
	if (CharacterGameplayInfo.bIsAirAttacking) return;
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle) return;

	CharacterGameplayInfo.bIsAirAttacking = true;
	ActionTrackingComponent->ResetComboCount();

	//LaunchCharcter is called by anim notify on upper attack animation

	if (IsValid(AssetHardPtrInfo.UpperAttackAnimMontage))
	{
		PlayAnimMontage(AssetHardPtrInfo.UpperAttackAnimMontage);
	}*/
}

void ACharacterBase::TryDownwardAttack()
{
	if (!GetCharacterMovement()->IsFalling()) return;

	CharacterGameplayInfo.bIsAirAttacking = false;
	CharacterGameplayInfo.bIsDownwardAttacking = true;
	ActionTrackingComponent->ResetComboCount();

	if (IsValid(AssetHardPtrInfo.DownwardAttackAnimMontage))
	{
		PlayAnimMontage(AssetHardPtrInfo.DownwardAttackAnimMontage);
		ActionTrackingComponent->CurrentComboType = EComboType::E_Jump;
		OnJumpAttackStarted.Broadcast();
	}
}

void ACharacterBase::TryDashAttack()
{
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (CharacterGameplayInfo.bIsAirAttacking || CharacterGameplayInfo.bIsDownwardAttacking) return;
	if (GetCharacterMovement()->IsFalling()) return;
	if (!IsValid(AssetHardPtrInfo.DashAttackAnimMontage)) return;
	if (GetVelocity().Length() <= CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed) * 0.75f) return;

	// Set action state
	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Attack;

	// Activate dash anim with interp to target location
	PlayAnimMontage(AssetHardPtrInfo.DashAttackAnimMontage);
	ActionTrackingComponent->CurrentComboType = EComboType::E_Dash;
	OnDashAttackStarted.Broadcast();
}

void ACharacterBase::DashAttack()
{
	//init local parameter
	const float dashLength = 250.0f;
	FVector targetLocation = GetActorLocation() + GetVelocity().GetSafeNormal() * dashLength;

	//check if there are any obstacle (wall, prop, enemy etc.)
	FHitResult hitResult;
	FCollisionQueryParams collisionQueryParams;
	collisionQueryParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(hitResult, HitSceneComponent->GetComponentLocation(), targetLocation, ECollisionChannel::ECC_Camera);
	targetLocation = hitResult.bBlockingHit ? hitResult.Location : targetLocation;

	SetLocationWithInterp(targetLocation, 2.5f);
}

bool ACharacterBase::TrySkill(int32 SkillID)
{
	if (!CharacterGameplayInfo.bCanAttack) return false;
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Skill
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Stun
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Die
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_KnockedDown) return false;


	/* @@@@@@@@@@ LEGACY SKILL CODE @@@@@@@@@@
	//스킬을 보유중인지 확인
	if (!SkillManagerComponentRef.Get()->IsSkillValid(SkillID))
	{
		GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("Skill is not Valid")), FColor::White);
		return false;
	}
	//스킬을 지닐 수 있는 무기와 현재 장비중인 무기가 일치하는지 확인
	ASkillBase* skillBase = SkillManagerComponentRef.Get()->GetOwnSkillBase(SkillID);
	if (skillBase->SkillStat.SkillWeaponStruct.bIsWeaponRequired)
	{
		if (!skillBase->SkillStat.SkillWeaponStruct.AvailableWeaponIDList.Contains(WeaponComponent->WeaponID))
		{
			GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("Does't have any weapon")), FColor::White);
			return false;
		}
	}
	if(skillBase->SkillState.bIsBlocked) return false;
	*/


	CharacterGameplayInfo.bCanAttack = false;
	SetActionState(ECharacterActionType::E_Skill);
	SkillManagerComponent->TryActivateSkill(SkillID);
	
	//Reset Combo
	ActionTrackingComponent->ResetComboCount();
	return true;
}

void ACharacterBase::Attack()
{
	const float attackSpeed = FMath::Min(1.5f, CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_NormalAttackSpeed) * WeaponComponent->WeaponStat.WeaponAtkSpeedRate);
	WeaponComponent->Attack();
}

void ACharacterBase::StopAttack()
{
	WeaponComponent->StopAttack();
}

void ACharacterBase::InitAsset(int32 NewCharacterID)
{
	AssetSoftPtrInfo = GetAssetSoftInfoWithID(NewCharacterID);
	AssetHardPtrInfo = FCharacterAssetHardInfo();
	
	if (!AssetSoftPtrInfo.CharacterMeshSoftPtr.IsNull())
	{
		TArray<FSoftObjectPath> AssetToStream;

		// Mesh 관련
		AssetToStream.AddUnique(AssetSoftPtrInfo.CharacterMeshSoftPtr.ToSoftObjectPath());

		if (!AssetSoftPtrInfo.NormalComboAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.NormalComboAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.NormalComboAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}
		if (!AssetSoftPtrInfo.JumpComboAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.JumpComboAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.JumpComboAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}
		if (!AssetSoftPtrInfo.DashComboAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.DashComboAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.DashComboAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		AssetToStream.AddUnique(AssetSoftPtrInfo.DownwardAttackAnimMontageSoftPtr.ToSoftObjectPath());
		AssetToStream.AddUnique(AssetSoftPtrInfo.DashAttackAnimMontageSoftPtr.ToSoftObjectPath());

		if (!AssetSoftPtrInfo.AirAttackAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.AirAttackAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.AirAttackAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.ShootAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.ShootAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.ShootAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.HitAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.HitAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.HitAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.RollAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.RollAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.RollAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.InvestigateAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.InvestigateAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.InvestigateAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.DieAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.DieAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.DieAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.PointMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.PointMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.PointMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.KnockdownAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.KnockdownAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.KnockdownAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		// VFX
		if (!AssetSoftPtrInfo.DebuffNiagaraEffectSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.DebuffNiagaraEffectSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.DebuffNiagaraEffectSoftPtrList[i].ToSoftObjectPath());
			}
		}

		// material
		if (!AssetSoftPtrInfo.DynamicMaterialSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.DynamicMaterialSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.DynamicMaterialSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.EmotionTextureSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.EmotionTextureSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.EmotionTextureSoftPtrList[i].ToSoftObjectPath());
			}
		}

		FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
		streamable.RequestAsyncLoad(AssetToStream, FStreamableDelegate::CreateUObject(this, &ACharacterBase::SetAsset));
	}
}

void ACharacterBase::SetAsset()
{
	//------Asset migrate to hard ref -------------------
	if (AssetSoftPtrInfo.CharacterMeshSoftPtr.IsNull() || !IsValid(AssetSoftPtrInfo.CharacterMeshSoftPtr.Get())) return;

	AssetHardPtrInfo.CharacterMesh = AssetSoftPtrInfo.CharacterMeshSoftPtr.Get();

	for (TSoftObjectPtr<UNiagaraSystem>& debuffNiagara : AssetSoftPtrInfo.DebuffNiagaraEffectSoftPtrList)
	{
		if (debuffNiagara.IsValid())
		{
			AssetHardPtrInfo.DebuffNiagaraEffectList.AddUnique(debuffNiagara.Get());
		}
	}
	for (TSoftObjectPtr<UMaterialInterface>& dynamicMaterial : AssetSoftPtrInfo.DynamicMaterialSoftPtrList)
	{
		if (dynamicMaterial.IsValid())
		{
			AssetHardPtrInfo.DynamicMaterialList.Add(dynamicMaterial.Get());
		}
	}
	for (TSoftObjectPtr<UTexture>& emotionTexture : AssetSoftPtrInfo.EmotionTextureSoftPtrList)
	{
		if (emotionTexture.IsValid())
		{
			AssetHardPtrInfo.EmotionTextureList.AddUnique(emotionTexture.Get());
		}
	}

	//------Asset Initialize  -------------------
	GetCapsuleComponent()->SetWorldRotation(FRotator::ZeroRotator);
	SetActorScale3D(AssetSoftPtrInfo.InitialCapsuleComponentScale);

	GetMesh()->SetSkeletalMesh(AssetSoftPtrInfo.CharacterMeshSoftPtr.Get());
	GetMesh()->SetRelativeLocation(AssetSoftPtrInfo.InitialLocation);
	GetMesh()->SetWorldRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeScale3D(AssetSoftPtrInfo.InitialScale);
	GetMesh()->SetAnimInstanceClass(AssetSoftPtrInfo.AnimBlueprint);
	GetMesh()->OverrideMaterials.Empty();

	//------ weapon component attachment ----------------------
	bool result = WeaponComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("Weapon_R"));
	if (!result)
	{
		//GEngine->AddOnScreenDebugMessage(0, 10, FColor::Yellow, FString("Failed"));
	}
	FLatentActionInfo latentInfo;
	latentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(WeaponComponent, FVector(0, 0, 0), FRotator(0, 0, 0)
		, 0, 0, 0, 0, EMoveComponentAction::Type::Move, latentInfo);

	//----Init other asset------------
	InitMaterialAsset();
	InitAnimAsset();
	InitSoundAsset();
	InitVFXAsset();
}

bool ACharacterBase::InitAnimAsset()
{
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.NormalComboAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.NormalComboAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.JumpComboAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.JumpComboAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.DashComboAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.DashComboAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	if (AssetSoftPtrInfo.DownwardAttackAnimMontageSoftPtr.IsValid())
	{
		AssetHardPtrInfo.DownwardAttackAnimMontage = AssetSoftPtrInfo.DownwardAttackAnimMontageSoftPtr.Get();
	}

	if (AssetSoftPtrInfo.DashAttackAnimMontageSoftPtr.IsValid())
	{
		AssetHardPtrInfo.DashAttackAnimMontage = AssetSoftPtrInfo.DashAttackAnimMontageSoftPtr.Get();
	}

	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.AirAttackAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.AirAttackAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.ShootAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.ShootAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.HitAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.HitAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.KnockdownAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.KnockdownAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.RollAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.RollAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.InvestigateAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.InvestigateAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.DieAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.DieAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.PointMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.PointMontageList.AddUnique(animSoftPtr.Get());
		}
	}

	return true;
}

void ACharacterBase::InitVFXAsset()
{
	if (!AssetSoftPtrInfo.DebuffNiagaraEffectSoftPtrList.IsEmpty())
	{
		for (TSoftObjectPtr<UNiagaraSystem>& vfx : AssetSoftPtrInfo.DebuffNiagaraEffectSoftPtrList)
		{
			if (vfx.IsValid())
				AssetHardPtrInfo.DebuffNiagaraEffectList.AddUnique(vfx.Get());
		}
	}
}

void ACharacterBase::InitSoundAsset(){}

void ACharacterBase::InitMaterialAsset()
{

	for (int8 matIdx = 0; matIdx < AssetHardPtrInfo.DynamicMaterialList.Num(); matIdx++)
	{
		if (IsValid(AssetHardPtrInfo.DynamicMaterialList[matIdx]))
		{
			GetMesh()->SetMaterial(matIdx, GetMesh()->CreateDynamicMaterialInstance(matIdx, AssetHardPtrInfo.DynamicMaterialList[matIdx]));
		}
	}
}

FCharacterAssetSoftInfo ACharacterBase::GetAssetSoftInfoWithID(const int32 TargetCharacterID)
{
	if (AssetDataInfoTable != nullptr)
	{
		FCharacterAssetSoftInfo* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetCharacterID);

		newInfo = AssetDataInfoTable->FindRow<FCharacterAssetSoftInfo>(FName(rowName), rowName);

		if (newInfo != nullptr) return *newInfo;
	}
	return FCharacterAssetSoftInfo();
}

void ACharacterBase::ActivateDebuffNiagara(uint8 DebuffType)
{
	if (!IsValid(BuffNiagaraEmitter) || DebuffType <= (uint8)ECharacterDebuffType::E_Temp) return;
	TArray<UNiagaraSystem*>& targetEmitterList = AssetHardPtrInfo.DebuffNiagaraEffectList;

	if (targetEmitterList.IsValidIndex(DebuffType) && targetEmitterList[DebuffType] != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACharacterBase::ActivateDebuffNiagara #2- %d"), DebuffType);
		BuffNiagaraEmitter->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
		BuffNiagaraEmitter->Deactivate();
		BuffNiagaraEmitter->SetAsset((targetEmitterList)[DebuffType], true);
		BuffNiagaraEmitter->Activate();
	}
}

void ACharacterBase::DeactivateBuffEffect()
{
	if (!IsValid(BuffNiagaraEmitter)) return;
	BuffNiagaraEmitter->SetAsset(nullptr, false);
	BuffNiagaraEmitter->Deactivate();
}

bool ACharacterBase::EquipWeapon(int32 NewWeaponID)
{
	if (WeaponComponent->WeaponID != 0) return false;
	WeaponComponent->InitWeapon(NewWeaponID);
	return true;
}

void ACharacterBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
	GetWorldTimerManager().ClearTimer(KnockDownDelayTimerHandle);
	GetWorldTimerManager().ClearTimer(HitCounterResetTimerHandle);
	GetWorldTimerManager().ClearTimer(LocationInterpHandle);
	GetWorldTimerManager().ClearTimer(AirAtkLocationUpdateHandle);
	AtkIntervalHandle.Invalidate();
	KnockDownDelayTimerHandle.Invalidate();
	HitCounterResetTimerHandle.Invalidate();
	LocationInterpHandle.Invalidate();
	AirAtkLocationUpdateHandle.Invalidate();
}
