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

FDelegateOnActionTrigger* ACharacterBase::GetActionTriggerDelegate(ECharacterActionType Type, bool bIsEndDelegate)
{
	FString actionTypeName = UEnum::GetValueAsString(Type);
	actionTypeName = actionTypeName.Replace(TEXT("E_"), TEXT(""));     // → "Attack"
	FString key;
	if (bIsEndDelegate)
	{
		key = FString::Printf(TEXT("On%sEnded"), *actionTypeName);
	}
	else
	{
		key = FString::Printf(TEXT("On%sStarted"), *actionTypeName);
	}
	if (ActionTriggerDelegateMap.Contains(FName(key)))
	{
		return ActionTriggerDelegateMap[FName(key)].Get();
	}
	
	UE_LOG(LogTemp, Error, TEXT("ACharacterBase::GetActionTriggerDelegate - 해당 액션 델리게이트가 존재하지 않습니다. ActionType: %s, IsEndDelegate: %s"), *actionTypeName, bIsEndDelegate ? TEXT("true") : TEXT("false"));
	return nullptr;
}

bool ACharacterBase::TryBroadcastActionTriggerDelegate(ECharacterActionType Type, bool bIsEndDelegate)
{
	FDelegateOnActionTrigger* actionDelegate = GetActionTriggerDelegate(Type, bIsEndDelegate);
	if (actionDelegate)
	{
		actionDelegate->Broadcast();
		return true;
	}
	return false;
}

void ACharacterBase::InitializeActionTriggerDelegateMap()
{ 
	//Init List, 액션 및 델리게이트 추가 시 필수
	//ECharacterActionType 순회

	ActionTriggerDelegateMap.Empty();

	//const int64 NumEnums = enumPtr->NumEnums();를 써도 될듯
	for (uint8 actionType = (uint8)(ECharacterActionType::E_NONE)+1;
		actionType < (uint8)(ECharacterActionType::COUNT); actionType++)
	{
		FString actionTypeName = UEnum::GetValueAsString((ECharacterActionType)actionType);
		actionTypeName = actionTypeName.Replace(TEXT("E_"), TEXT(""));     // → "Attack"

		const FString startKey = FString::Printf(TEXT("On%sStarted"), *actionTypeName);
		const FString endKey = FString::Printf(TEXT("On%sEnded"), *actionTypeName);
		

		ActionTriggerDelegateMap.Add(FName(startKey), MakeUnique<FDelegateOnActionTrigger>());
		ActionTriggerDelegateMap.Add(FName(endKey), MakeUnique<FDelegateOnActionTrigger>());
	}
}

void ACharacterBase::ResetAtkIntervalTimer()
{
	CharacterGameplayInfo.bCanAttack = true;
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
}

void ACharacterBase::SetLocationWithInterp(FVector StartLocation, FVector TargetLocation, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeList, float InterpSpeed, const bool bAutoReset, const bool bUseSafeEndLocation, const bool bUseSafeInterpBlock, const bool bIgnoreZAxis, const bool bDrawDebugInfo)
{
	FTimerDelegate updateFunctionDelegate, safeCheckDelegate;
	
	if (bUseSafeEndLocation)
	{
		FHitResult hitResult;
		UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), StartLocation, TargetLocation, 30.0f
			, ObjectTypeList, true, { this }, bDrawDebugInfo ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hitResult, true);

		if (hitResult.bBlockingHit)
		{
			FVector direction = (TargetLocation - GetActorLocation());
			TargetLocation = hitResult.ImpactPoint - direction.GetSafeNormal() * FMath::Clamp(direction.Length() * 0.1f, 0.0f, 100.0f);
		}
		if (bDrawDebugInfo)
		{
			DrawDebugLine(GetWorld(), StartLocation, TargetLocation, FColor::Red, false, 5.0f);
			DrawDebugSphere(GetWorld(), hitResult.ImpactPoint, 10.0f, 12, FColor::Red, false, 5.0f);
			DrawDebugSphere(GetWorld(), TargetLocation, 10.0f, 12, FColor::Yellow, false, 5.0f);
			DrawDebugSphere(GetWorld(), StartLocation, 10.0f, 12, FColor::Orange, false, 5.0f);
		}
	}

	//위치 업데이트 이벤트 바인딩
	OnBeginLocationInterp.Broadcast();
	updateFunctionDelegate.BindUFunction(this, FName("UpdateLocation"), TargetLocation, InterpSpeed, bAutoReset, bIgnoreZAxis);
	
	ResetLocationInterpTimer();
	GetWorld()->GetTimerManager().SetTimer(LocationInterpHandle, updateFunctionDelegate, 0.01f, true);

	if (bUseSafeInterpBlock)
	{
		//벽뚫 방지용
		safeCheckDelegate.BindUFunction(this, FName("CheckSafeLocation"), TargetLocation);
		GetWorld()->GetTimerManager().SetTimer(InterpSafeCheckHandle, safeCheckDelegate, 0.1f, true);
	}
}

void ACharacterBase::ResetLocationInterpTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);
	GetWorld()->GetTimerManager().ClearTimer(InterpSafeCheckHandle);
	LocationInterpHandle.Invalidate();
	InterpSafeCheckHandle.Invalidate();
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

void ACharacterBase::UpdateLocation(const FVector TargetValue, const float InterpSpeed, const bool bAutoReset, const bool bIgnoreZAxis)
{
	FVector currentLocation = GetActorLocation();
	FVector targetLocation = TargetValue;

	if (bIgnoreZAxis) 
	{
		currentLocation.Z = targetLocation.Z; //Z축은 무시
	}
	if (currentLocation.Equals(targetLocation, GetCharacterMovement()->IsFalling() ? FMath::Abs(GetVelocity().Z) * 0.2f : 50.0f))
	{
		ResetLocationInterpTimer();
		OnEndLocationInterp.Broadcast();
		if (bAutoReset)
		{
			//SetLocationWithInterp(GetActorLocation() , InterpSpeed * 1.5f, false, );
		}
	}
	currentLocation = FMath::VInterpTo(currentLocation, targetLocation, 0.1f, InterpSpeed);
	if (bIgnoreZAxis)
	{
		currentLocation.Z = GetActorLocation().Z; //Z축은 무시
	}
	SetActorLocation(currentLocation, false, nullptr, ETeleportType::None);

	//Prevent pitch turns
	FRotator newRotation = GetActorRotation();
	newRotation.Pitch = 0.0f;
	SetActorRotation(newRotation);
}

void ACharacterBase::CheckSafeLocation(const FVector TargetValue)
{
	//capsule single overlap check for ECC_Pawn and ECC_WorldStatic and ECC_WorldDynamic
	TArray<FOverlapResult> overlapResults;
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);

	//ignore self
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);

	bool bIsOverlapping = GetWorld()->OverlapMultiByObjectType(
		overlapResults,
		GetActorLocation(),
		FQuat::Identity,
		objectQueryParams,
		FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.25f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
		queryParams
	);
	if (bIsOverlapping)
	{
		ResetLocationInterpTimer();
		DrawDebugCapsule(GetWorld(), GetActorLocation(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), GetCapsuleComponent()->GetScaledCapsuleRadius(), FQuat::Identity, FColor::Red, false, 5.0f);
	}
	else
	{
		DrawDebugCapsule(GetWorld(), GetActorLocation(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), GetCapsuleComponent()->GetScaledCapsuleRadius(), FQuat::Identity, FColor::Green, false, 5.0f);
	}
}

void ACharacterBase::OnPlayerLanded(const FHitResult& Hit)
{
	CharacterGameplayInfo.bIsAirAttacking = false;
	CharacterGameplayInfo.bIsDownwardAttacking = false;
	
	GetActionTriggerDelegate(ECharacterActionType::E_Jump, true)->Broadcast();

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
}

void ACharacterBase::OnSkillDeactivated(FSkillInfo PrevSkillInfo)
{
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Skill, true);
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
				nextAnimIdx = (ActionTrackingComponent->CurrentComboCount) % AssetHardPtrInfo.JumpComboAnimMontageList.Num();
				targetAnimList = AssetHardPtrInfo.JumpComboAnimMontageList;
				ActionTrackingComponent->CurrentComboType = EComboType::E_Jump;
				return targetAnimList;
			}
		}
		//check melee anim type
		if (AssetHardPtrInfo.NormalComboAnimMontageList.Num() > 0)
		{
			nextAnimIdx = (ActionTrackingComponent->CurrentComboCount) % AssetHardPtrInfo.NormalComboAnimMontageList.Num();
			targetAnimList = AssetHardPtrInfo.NormalComboAnimMontageList;
		}
		break;

	case EWeaponType::E_Shoot:
		if (AssetHardPtrInfo.ShootAnimMontageList.Num() > 0)
		{
			nextAnimIdx = (ActionTrackingComponent->CurrentComboCount) % AssetHardPtrInfo.ShootAnimMontageList.Num();
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

void ACharacterBase::SetCharacterGameplayInfo(FCharacterGameplayInfo NewGameplayInfo)
{
	if (!NewGameplayInfo.IsValid()) return;
	
	CharacterGameplayInfo = NewGameplayInfo;
	CharacterID = NewGameplayInfo.CharacterID;
	InitAsset(CharacterID);

	CharacterGameplayInfo.UpdateTotalValues();
	CharacterGameplayInfo.bCanAttack = true;
	CharacterGameplayInfo.bCanRoll = true;
	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Idle;

	GetCharacterMovement()->MaxWalkSpeed = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed);
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
	if (!bForceReset && CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Die) return;
	if (!bForceReset && CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Stun) return;	

	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Idle;

	//Reset Location Interp Timer Handle
	//250609 넉백 코드를 위해 비활성화
	//GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);

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

	// =========== 데미지 계산 로직 ==============
	// 1. 방어력(Defense) 적용: 최대 50%까지 데미지 삭감
	float defenseValue = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_Defense);
	float defenseBonus = FMath::Min(defenseValue * 0.5f, 0.5f);
	DamageAmount *= (1.0f - defenseBonus);

	// 2. 피해 감소(DamageReduction) 적용: 남은 데미지를 비율만큼 추가 압축 / Player 에게만 적용
	if (ActorHasTag("Player"))
	{
		float damageReduc = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_DamageReduction);
		DamageAmount *= (1.0f - FMath::Min(damageReduc, 0.9f)); // 최대 90% 제한
	}

	// 3. 최종 데미지 보정: 최소 1의 피해는 보장
	DamageAmount = FMath::Max(DamageAmount, 1.0f);
	// ========================================

	CharacterGameplayInfo.CurrentHP = CharacterGameplayInfo.CurrentHP - DamageAmount;
	newClampedHealthValue = CharacterGameplayInfo.CurrentHP = FMath::Max(0.0f, CharacterGameplayInfo.CurrentHP);
	
	oldClampedHealthValue = UKismetMathLibrary::MapRangeClamped(oldClampedHealthValue, 0.0f, totalHealthValue, 0.0f, 100.0f);
	newClampedHealthValue = UKismetMathLibrary::MapRangeClamped(newClampedHealthValue, 0.0f, totalHealthValue, 0.0f, 100.0f);

	OnHealthChanged.Broadcast(oldClampedHealthValue, newClampedHealthValue);
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Hit, false);

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
	if (!DamageCauser->ActorHasTag("Character")) return DamageAmount;

	// ====== move to enemy's Hit scene location ======================
	//Update EnemyCharacter's Rotation
	FVector location = HitSceneComponent->GetComponentLocation();
	FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(DamageCauser->GetActorLocation(), GetActorLocation());
	newRotation.Pitch = newRotation.Roll = 0.0f;

	//Reset Location Interp Timer Handle
	//250609 넉백 코드를 위해 비활성화
	//GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);

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

	TakeDamage(DamageAmount, FDamageEvent(), Causer->GetInstigatorController(), this);
	return DamageAmount;
}

bool ACharacterBase::ApplyDebuffStack(FDebuffInfoStruct DebuffInfo, AActor* Causer)
{
	if (!IsValid(Causer) || Causer->IsActorBeingDestroyed()) return false;
	if (GetIsActionActive(ECharacterActionType::E_Die)) return false;

	float stackAmount = DebuffInfo.StackAmount;

	//Resist 스탯 적용 (다른 곳에서 하지 않기!!!!!!!!!!)
	switch (DebuffInfo.Type)
	{
	case ECharacterDebuffType::E_FrameDrop:
		stackAmount *= (1.0f - FMath::Clamp(GetStatTotalValue(ECharacterStatType::E_SlowResist), 0.0f, 1.0f));	//FrameDrop Resist 추가 대신 E_SlowResist로 대체
		break;
	case ECharacterDebuffType::E_Poison:
		stackAmount *= (1.0f - FMath::Clamp(GetStatTotalValue(ECharacterStatType::E_PoisonResist), 0.0f, 1.0f));
		break;
	case ECharacterDebuffType::E_Burn:
		stackAmount *= (1.0f - FMath::Clamp(GetStatTotalValue(ECharacterStatType::E_BurnResist), 0.0f, 1.0f));
		break;
	}

	DebuffInfo.StackAmount = stackAmount;

	if (DebuffInfo.Type == ECharacterDebuffType::E_Stun)
	{
		SkillManagerComponent->DeactivateCurrentSkill(); //Force Deactivate Skill
	}

	DebuffManagerComponent->ApplyDebuffStack(DebuffInfo, Causer);
	return true;
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

	HealAmount = (HealAmount * (1.0f + GetStatTotalValue(ECharacterStatType::E_HealItemPerformance)));
	CharacterGameplayInfo.CurrentHP += HealAmount;
	newClampedHealthValue = CharacterGameplayInfo.CurrentHP = FMath::Min(totalHealthValue, CharacterGameplayInfo.CurrentHP);
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Hit, false);
	
	//실제 회복된 양을 계산
	CharacterGameplayInfo.TotalHealAmount += (newClampedHealthValue - oldClampedHealthValue);

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

	if (Target->ActorHasTag("Enemy") && !Cast<ACharacterBase>(Target)->GetCharacterGameplayInfo().bIsInvincibility)
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

	//무적 처리를 하고, Movement를 비활성화
	CharacterGameplayInfo.bIsInvincibility = true;
	bUseControllerRotationYaw = false;

	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Die, false);

	if (AssetHardPtrInfo.DieAnimMontageList.Num() > 0
		&& IsValid(AssetHardPtrInfo.DieAnimMontageList[0]))
	{
		PlayAnimMontage(AssetHardPtrInfo.DieAnimMontageList[0]);
	}
}

void ACharacterBase::Revive()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	ResetActionState(true);
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Revive, false);

	TakeHeal(CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MaxHealth));

	//Remove invincibility after 3 seconds using lambda timer
	GetWorldTimerManager().SetTimer(RemoveInvincibilityDelegate, FTimerDelegate::CreateLambda([this]()
		{
			if (IsValid(this)) this->CharacterGameplayInfo.bIsInvincibility = false;
		}), 3.0f, false);
}

void ACharacterBase::TryAttack()
{
	if (GetWorldTimerManager().IsTimerActive(AtkIntervalHandle)) return;
	if (!CharacterGameplayInfo.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return;

	int32 nextAnimIdx = ActionTrackingComponent->CurrentComboCount;
	int32 maxComboIdx = ActionTrackingComponent->CurrentComboType == EComboType::E_Normal ? GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_MaxNormalComboIdx)
						: ActionTrackingComponent->CurrentComboType == EComboType::E_Dash ? GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_MaxDashComboIdx)
						: ActionTrackingComponent->CurrentComboType == EComboType::E_Jump ? GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_MaxAerialComboIdx)
						: 0;
	maxComboIdx = maxComboIdx == 0 ? 1e9 : maxComboIdx; //0이면 무한으로 처리
	nextAnimIdx = nextAnimIdx % maxComboIdx;
	if (nextAnimIdx == 0 && (ActorHasTag(TEXT("Player")) && (!ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_Attack)
		|| !ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_DashAttack)
		|| !ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_JumpAttack))))

	{
		return;
	}

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
			TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Attack, false);
			break;
		case EComboType::E_Dash:
			TryBroadcastActionTriggerDelegate(ECharacterActionType::E_DashAttack, false);
			break;
		case EComboType::E_Jump:
			TryBroadcastActionTriggerDelegate(ECharacterActionType::E_JumpAttack, false);
			break;
		}
	}
}

bool ACharacterBase::TrySkill(int32 SkillID)
{
	if (ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_Skill)
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
		TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Skill, false);
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
	FTimerHandle attackEndTimerHandle;
	GetWorldTimerManager().SetTimer(attackEndTimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		if (IsValid(this))
		{
			this->TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Attack, true);
			this->TryBroadcastActionTriggerDelegate(ECharacterActionType::E_DashAttack, true);
			this->TryBroadcastActionTriggerDelegate(ECharacterActionType::E_JumpAttack, true);
		}
	}), 0.65f, false);
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
	GetCapsuleComponent()->SetCapsuleHalfHeight(AssetSoftPtrInfo.CapsuleHalfHeight);
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
	AssetHardPtrInfo.DynamicMaterialInstanceList.Empty();
	for (int8 matIdx = 0; matIdx < AssetHardPtrInfo.DynamicMaterialList.Num(); matIdx++)
	{
		if (IsValid(AssetHardPtrInfo.DynamicMaterialList[matIdx]))
		{
			AssetHardPtrInfo.DynamicMaterialInstanceList.Add(GetMesh()->CreateDynamicMaterialInstance(matIdx, AssetHardPtrInfo.DynamicMaterialList[matIdx]));
			GetMesh()->SetMaterial(matIdx, AssetHardPtrInfo.DynamicMaterialInstanceList.Last());
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
