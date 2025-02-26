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
	WeaponComponent->SetupAttachment(GetMesh());

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
	SkillManagerComponent->OnSkillDeactivated.AddDynamic(this, &ACharacterBase::OnSkillDeactivated);
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
	ActionTriggerDelegateMap.Add("OnAimStarted", &OnAimStarted);
	ActionTriggerDelegateMap.Add("OnAimEnd", &OnAimEnded);
	ActionTriggerDelegateMap.Add("OnShootStarted", &OnShootStarted);
	ActionTriggerDelegateMap.Add("OnInteractStarted", &OnInteractStarted);
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

void ACharacterBase::SetLocationWithInterp(FVector NewValue, float InterpSpeed, const bool bAutoReset, const bool bUseSafeInterp, const bool bDrawDebugInfo)
{
	FTimerDelegate updateFunctionDelegate;

	//안정성 강화 코드 (벽뚫 방지)
	if (bUseSafeInterp)
	{
		FHitResult hitResult;
		FCollisionObjectQueryParams objectQueryParams;
		objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
		objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
		GetWorld()->LineTraceSingleByObjectType(hitResult, GetActorLocation(), NewValue, objectQueryParams, FCollisionQueryParams::DefaultQueryParam);
		if (hitResult.bBlockingHit)
		{
			FVector direction = (NewValue - GetActorLocation());
			NewValue = hitResult.ImpactPoint - direction.GetSafeNormal() * FMath::Clamp(direction.Length() * 0.1f, 0.0f, 100.0f);
		}
		if (bDrawDebugInfo)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), NewValue, FColor::Red, false, 5.0f);
			DrawDebugSphere(GetWorld(), hitResult.ImpactPoint, 10.0f, 12, FColor::Red, false, 5.0f);
		}
	}
	if (bDrawDebugInfo)
	{
		DrawDebugSphere(GetWorld(), NewValue, 10.0f, 12, FColor::Yellow, false, 5.0f);
		DrawDebugSphere(GetWorld(), GetActorLocation(), 10.0f, 12, FColor::Orange, false, 5.0f);
	}

	//위치 업데이트 이벤트 바인딩
	OnBeginLocationInterp.Broadcast();
	updateFunctionDelegate.BindUFunction(this, FName("UpdateLocation"), NewValue, InterpSpeed, bAutoReset);

	ResetLocationInterpTimer();
	GetWorld()->GetTimerManager().SetTimer(LocationInterpHandle, updateFunctionDelegate, 0.01f, true);
}

void ACharacterBase::ResetLocationInterpTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);
}

void ACharacterBase::PlayHitAnimMontage()
{
	if (GetIsActionActive(ECharacterActionType::E_Skill)) return;
	
	if (ActorHasTag("Enemy")) 
	{
		AAIControllerBase* aiControllerRef = nullptr;
		aiControllerRef = Cast<AAIControllerBase>(Controller);
		if (!IsValid(aiControllerRef) && aiControllerRef->GetBehaviorState() != EAIBehaviorType::E_Skill) return;
	}

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

void ACharacterBase::UpdateLocation(const FVector TargetValue, const float InterpSpeed, const bool bAutoReset)
{
	FVector currentLocation = GetActorLocation();
	UE_LOG(LogWeapon, Warning, TEXT("Update Location) Falling : %d, Error : %d"), (int32)(GetCharacterMovement()->IsFalling()), (int32)(GetCharacterMovement()->IsFalling() ? GetVelocity().Length() * 0.02f : 30.0f));
	if (currentLocation.Equals(TargetValue, GetCharacterMovement()->IsFalling() ? GetVelocity().Length() * 0.02f : 30.0f))
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
	
	//Prevent pitch turns
	FRotator newRotation = GetActorRotation();
	newRotation.Pitch = 0.0f;
	SetActorRotation(newRotation);
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
	}
	//Test code for knockdown on ground event
	//UE_LOG(LogTemp, Warning, TEXT("$Land %s  / Speed %.2lf$"), *(this->GetName()), this->GetVelocity().Length());
}

void ACharacterBase::OnSkillDeactivated(FSkillInfo PrevSkillInfo)
{
	OnSkillEnded.Broadcast();
	ResetActionState();
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
			if (GetCharacterMovement()->IsFalling() && AssetHardPtrInfo.JumpComboAnimMontageList.Num() > 0)
			{
				nextAnimIdx = ActionTrackingComponent->CurrentComboCount % AssetHardPtrInfo.JumpComboAnimMontageList.Num();
				targetAnimList = AssetHardPtrInfo.JumpComboAnimMontageList;
				ActionTrackingComponent->CurrentComboType = EComboType::E_Jump;
				return targetAnimList;
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
	if (NewGameplayInfo.bUseDefaultStat || CharacterGameplayInfo.bUseDefaultStat)
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
	if (!GamemodeRef.IsValid() || CharacterGameplayInfo.bIsInvincibility) return false;

	//Resist 스탯 적용
	switch (DebuffInfo.Type)
	{
	case ECharacterDebuffType::E_Stun:
		DebuffInfo.TotalTime *= (1.0f - FMath::Clamp(GetStatTotalValue(ECharacterStatType::E_StunResist), 0.0f, 1.0f));
		break;
	case ECharacterDebuffType::E_Slow:
	case ECharacterDebuffType::E_Poison:
	case ECharacterDebuffType::E_Burn:
		DebuffInfo.Variable *= (1.0f - FMath::Clamp(GetStatTotalValue(ECharacterStatType::E_StunResist), 0.0f, 1.0f));
		break;
	}
	bool result = DebuffManagerComponent->SetDebuffTimer(DebuffInfo, Causer);
	if (result && DebuffInfo.Type == ECharacterDebuffType::E_Stun)
	{
		SkillManagerComponent->DeactivateCurrentSkill(); //Force Deactivate Skill
	}

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

	//Calculate attack speed
	float attackSpeed = 0.0f;
	switch (ActionTrackingComponent->CurrentComboType)
	{
	case EComboType::E_Normal:
		attackSpeed = GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_NormalAttackSpeed);
		break;
	case EComboType::E_Dash:
		attackSpeed = GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_DashAttackSpeed);
		break;
	case EComboType::E_Jump:
		attackSpeed = GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_JumpAttackSpeed);
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("ACharacterBase::TryAttack() for %s, ComboType is not valid"), *UKismetSystemLibrary::GetDisplayName(this));
	}
	attackSpeed = FMath::Clamp(attackSpeed * WeaponComponent->GetWeaponStat().WeaponAtkSpeedRate, 0.2f, 1.5f);

	//Choose animation which fit battle situation
	TArray <UAnimMontage*> targetAnimList = GetTargetMeleeAnimMontageList();

	//Error exception
	if (targetAnimList.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("ACharacterBase::TryAttack() - ID : %d, Combo type %d, anim montage list is empty"), CharacterID, (int32)ActionTrackingComponent->CurrentComboType);
		return;
	}
		
	// 언젠가 MainCharacter로 이주 필요
	if (targetAnimList.IsValidIndex(nextAnimIdx) && IsValid(targetAnimList[nextAnimIdx]))
	{
		PlayAnimMontage(targetAnimList[nextAnimIdx], attackSpeed + 0.25f);
		CharacterGameplayInfo.bCanAttack = false; //공격간 Delay,Interval 조절을 위해 세팅
		CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Attack;
		
		switch (ActionTrackingComponent->CurrentComboType)
		{
		case EComboType::E_Normal:
			OnAttackStarted.Broadcast();
			break;
		case EComboType::E_Dash:
			OnDashAttackStarted.Broadcast();
			break;
		case EComboType::E_Jump:
			OnJumpAttackStarted.Broadcast();
			break;
		}
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

	// Get Total Stat Value
	float attackSpeed = GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_DashAttackSpeed);
	attackSpeed = FMath::Clamp(attackSpeed * WeaponComponent->GetWeaponStat().WeaponAtkSpeedRate, 0.2f, 1.5f);

	// Activate dash anim with interp to target location
	OnDashAttackStarted.Broadcast();
	PlayAnimMontage(AssetHardPtrInfo.DashAttackAnimMontage, attackSpeed);
	ActionTrackingComponent->CurrentComboType = EComboType::E_Dash;
}

bool ACharacterBase::TrySkill(int32 SkillID)
{
	if (ActionTrackingComponent->GetIsActionInProgress("Skill")
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Stun
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Die
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_KnockedDown) return false;

	bool result = SkillManagerComponent->TryActivateSkill(SkillID);
	if (result)
	{
		//Reset Combo
		CharacterGameplayInfo.bCanAttack = false;
		SetActionState(ECharacterActionType::E_Skill);

		ActionTrackingComponent->ResetComboCount();
		OnSkillStarted.Broadcast();
	}

	return result;
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
	if (IsValid(GetMesh()->GetSkeletalMeshAsset()))
	{
		bool result = WeaponComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("Weapon_R"));
		if (!result)
		{
			UE_LOG(LogTemp, Error, TEXT("ACharacterBase::SetAsset() for %s,  Weapon attachment is falied"), *UKismetSystemLibrary::GetDisplayName(this));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ACharacterBase::SetAsset() for %s, skeletal mesh is not valid"), *UKismetSystemLibrary::GetDisplayName(this));
	}

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
		//BuffNiagaraEmitter->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
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
