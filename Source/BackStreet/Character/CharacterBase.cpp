#include "CharacterBase.h"
#include "./Component/DebuffManagerComponent.h"
#include "./Component/TargetingManagerComponent.h"
#include "./Component/WeaponComponentBase.h"
#include "./Component/SkillManagerComponent.h"
#include "../Item/Weapon/Ranged/RangedCombatManager.h"
#include "../Global/BackStreetGameModeBase.h"
#include "../System/AssetSystem/AssetManagerBase.h"
#include "../System/SkillSystem/SkillBase.h"
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
	TargetingManagerComponent = CreateDefaultSubobject<UTargetingManagerComponent>(TEXT("TARGETING_MANAGER"));

	WeaponComponent = CreateDefaultSubobject<UWeaponComponentBase>(TEXT("WeaponBase"));
	WeaponComponent->SetupAttachment(GetMesh());

	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CharacterID = AssetSoftPtrInfo.CharacterID;
	InitCharacterState();

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

void ACharacterBase::ResetAtkIntervalTimer()
{
	CharacterState.bCanAttack = true;
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
}

void ACharacterBase::SetLocationWithInterp(FVector NewValue, float InterpSpeed, const bool bAutoReset)
{
	FTimerDelegate updateFunctionDelegate;

	//Binding the function with specific values
	updateFunctionDelegate.BindUFunction(this, FName("UpdateLocation"), NewValue, InterpSpeed, bAutoReset);

	//Calling MyUsefulFunction after 5 seconds without looping
	ResetLocationInterpTimer();
	GetWorld()->GetTimerManager().SetTimer(LocationInterpHandle, updateFunctionDelegate, 0.01f, true);
}

void ACharacterBase::ResetLocationInterpTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);
}

void ACharacterBase::SetAirAtkLocationUpdateTimer()
{
	GetWorldTimerManager().SetTimer(AirAtkLocationUpdateHandle, this, &ACharacterBase::SetAirAttackLocation, 0.1, true);
}

void ACharacterBase::ResetAirAtkLocationUpdateTimer()
{
	CharacterState.bIsAirAttacking = false;
	GetWorldTimerManager().ClearTimer(AirAtkLocationUpdateHandle);
	AirAtkLocationUpdateHandle.Invalidate();
}

void ACharacterBase::UpdateLocation(const FVector TargetValue, const float InterpSpeed, const bool bAutoReset)
{
	FVector currentLocation = GetActorLocation();
	if (currentLocation.Equals(TargetValue, 10.0f))
	{
		GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);
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
	CharacterState.bIsAirAttacking = false;
	CharacterState.bIsDownwardAttacking = false;

	ResetActionState();

	if (CharacterState.CharacterActionState == ECharacterActionType::E_Die)
	{
		Die();
	}

	//damager side 
	if (this->GetVelocity().Length() >= 3500.0f)
	{
		GamemodeRef.Get()->ActivateSlowHitEffect(0.2f);
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
	CharacterState.HitCounter = 0;
}

void ACharacterBase::KnockDown()
{
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Die) return;
	ResetHitCounter();

	CharacterState.CharacterActionState = ECharacterActionType::E_KnockedDown;

	GetWorldTimerManager().ClearTimer(KnockDownDelayTimerHandle);
	KnockDownDelayTimerHandle.Invalidate();

	
	FTimerDelegate KnockDownDelegate;
	KnockDownDelegate.BindUFunction(this, "StandUp");
	GetWorldTimerManager().SetTimer(KnockDownAnimMontageHandle, KnockDownDelegate, 1.0f, false, 2.0f);
	
}

void ACharacterBase::StandUp()
{
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Die) return;
	CharacterState.CharacterActionState = ECharacterActionType::E_Idle;
	GetWorldTimerManager().ClearTimer(KnockDownAnimMontageHandle);
}

void ACharacterBase::InitCharacterState()
{
	CharacterState.CurrentHP = CharacterStat.DefaultHP;
	CharacterState.bCanAttack = true;
	CharacterState.CharacterActionState = ECharacterActionType::E_Idle;

	UpdateCharacterStat(CharacterStat);
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

void ACharacterBase::UpdateCharacterStatAndState(FCharacterStatStruct NewStat, FCharacterStateStruct NewState)
{
	UpdateCharacterState(NewState);
	UpdateCharacterStat(NewStat);
}

void ACharacterBase::UpdateCharacterStat(FCharacterStatStruct NewStat)
{
	CharacterStat = NewStat;

	//Update Character State's total property 
	const float hpRate = CharacterState.CurrentHP / CharacterState.TotalHP;
	const float oldTotalHP = CharacterState.TotalHP;
	const float oldCurrentHP = CharacterState.CurrentHP;
	CharacterState.TotalHP = GetTotalStatValue(CharacterStat.DefaultHP, CharacterState.AbilityHP, CharacterState.SkillHP, CharacterState.DebuffHP);
	CharacterState.CurrentHP = hpRate > 1.0f ? CharacterState.CurrentHP : CharacterState.TotalHP * hpRate;
	CharacterState.TotalAttack = GetTotalStatValue(CharacterStat.DefaultAttack, CharacterState.AbilityAttack, CharacterState.SkillAttack, CharacterState.DebuffAttack);
	CharacterState.TotalDefense = GetTotalStatValue(CharacterStat.DefaultDefense, CharacterState.AbilityDefense, CharacterState.SkillDefense, CharacterState.DebuffDefense);
	CharacterState.TotalMoveSpeed = GetTotalStatValue(CharacterStat.DefaultMoveSpeed, CharacterState.AbilityMoveSpeed, CharacterState.SkillMoveSpeed, CharacterState.DebuffMoveSpeed);
	CharacterState.TotalAttackSpeed = GetTotalStatValue(CharacterStat.DefaultAttackSpeed, CharacterState.AbilityAttackSpeed, CharacterState.SkillAttackSpeed, CharacterState.DebuffAttackSpeed);

	//Update movement speed
	GetCharacterMovement()->MaxWalkSpeed = CharacterState.TotalMoveSpeed;
}

void ACharacterBase::UpdateCharacterState(FCharacterStateStruct NewState)
{
	CharacterState = NewState;
}

void ACharacterBase::UpdateWeaponStat(FWeaponStatStruct NewStat)
{
	WeaponComponent->WeaponStat = NewStat;
}

void ACharacterBase::ResetActionState(bool bForceReset)
{
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Die
		|| CharacterState.CharacterActionState == ECharacterActionType::E_KnockedDown) return;
	if (!bForceReset && (CharacterState.CharacterActionState == ECharacterActionType::E_Stun
		|| CharacterState.CharacterActionState == ECharacterActionType::E_Reload)) return;

	CharacterState.CharacterActionState = ECharacterActionType::E_Idle;

	//Reset Location Interp Timer Handle
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);

	FWeaponStatStruct currWeaponStat = this->WeaponComponent->GetWeaponStat();
	this->WeaponComponent->SetWeaponStat(currWeaponStat);
	StopAttack();

	if (!GetWorldTimerManager().IsTimerActive(AtkIntervalHandle))
	{
		CharacterState.bCanAttack = true;
	}
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsValid(DamageCauser) || CharacterStat.bIsInvincibility || DamageAmount <= 0.0f) return 0.0f;
	//if (GetIsActionActive(ECharacterActionType::E_Die)) return 0.0f;

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// ======= Damage & Die event ===============================
	CharacterState.CurrentHP = CharacterState.CurrentHP - DamageAmount * (1.0f - FMath::Clamp(0.5f * CharacterState.TotalDefense, 0.0f, 0.5f));
	CharacterState.CurrentHP = FMath::Max(0.0f, CharacterState.CurrentHP);
	if (CharacterState.CurrentHP == 0.0f)
	{
		CharacterState.CharacterActionState = ECharacterActionType::E_Die;
		if (!GetCharacterMovement()->IsFalling())
		{
			Die();
		}
	}
	else if (this->CharacterState.CharacterActionState != ECharacterActionType::E_Skill && !this->CharacterState.bIsAirAttacking)
	{
		const int32 randomIdx = UKismetMathLibrary::RandomIntegerInRange(0, AssetSoftPtrInfo.HitAnimMontageSoftPtrList.Num() - 1);
		if (AssetHardPtrInfo.HitAnimMontageList.Num() > 0 && IsValid(AssetHardPtrInfo.HitAnimMontageList[randomIdx]))
		{
			PlayAnimMontage(AssetHardPtrInfo.HitAnimMontageList[randomIdx]);
		}
	}

	//if causer is not character(like resource manager), return func immediately
	if (!DamageCauser->ActorHasTag("Character")) return 0.0f;

	// ====== move to enemy's Hit scene location ======================
	//Update EnemyCharacter's Rotation
	FVector location = HitSceneComponent->GetComponentLocation();
	FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(DamageCauser->GetActorLocation(), GetActorLocation());
	newRotation.Pitch = newRotation.Roll = 0.0f;

	if (DamageCauser->ActorHasTag("Player"))
	{
		ACharacterBase* causerTarget = Cast<ACharacterBase>(DamageCauser)->TargetingManagerComponent->GetTargetedCharacter();
		if (IsValid(causerTarget) && causerTarget == this)
		{
			Cast<ACharacterBase>(DamageCauser)->SetActorRotation(newRotation);
			newRotation.Yaw += 180.0f;
			SetActorRotation(newRotation);
		}
	}

	//Reset Location Interp Timer Handle
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);

	// ====== Hit Counter & Knock Down Check ===========================
	CharacterState.HitCounter += 1;

	// Set hit counter reset event using retriggable timer
	GetWorldTimerManager().ClearTimer(HitCounterResetTimerHandle);
	HitCounterResetTimerHandle.Invalidate();
	GetWorldTimerManager().SetTimer(HitCounterResetTimerHandle, this, &ACharacterBase::ResetHitCounter, 1.0f, false, 1.0f);

	// Check knock down condition and set knock down event using retriggable timer
	if (Cast<ACharacterBase>(DamageCauser)->WeaponComponent->GetWeaponStat().FinalImpactStrength > 0 && Cast<ACharacterBase>(DamageCauser)->WeaponComponent->GetIsFinalCombo())
	{
		KnockDown();
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
	TakeDamage(DamageAmount, FDamageEvent(), nullptr, Causer);
	return DamageAmount;
}

void ACharacterBase::SetActionState(ECharacterActionType Type)
{
	CharacterState.CharacterActionState = Type;
	return;
}

void ACharacterBase::TakeHeal(float HealAmount, bool bIsTimerEvent, uint8 BuffDebuffType)
{
	CharacterState.CurrentHP += HealAmount;
	CharacterState.CurrentHP = FMath::Min(CharacterStat.DefaultHP, CharacterState.CurrentHP);
	OnTakeDamage.Broadcast();
	return;
}

void ACharacterBase::ApplyKnockBack(AActor* Target, float Strength)
{
	if (!IsValid(Target) || Target->IsActorBeingDestroyed()) return;
	if (Cast<ACharacterBase>(Target)->GetCharacterMovement()->IsFalling()) return;
	if (GetIsActionActive(ECharacterActionType::E_Die)) return;

	FVector knockBackDirection = Target->GetActorLocation() - GetActorLocation();
	knockBackDirection = knockBackDirection.GetSafeNormal();
	knockBackDirection *= Strength;
	knockBackDirection.Z = 1.0f;

	//GetCharacterMovement()->AddImpulse(knockBackDirection);
	Cast<ACharacterBase>(Target)->LaunchCharacter(knockBackDirection, true, false);
}

void ACharacterBase::Die()
{
	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	//모든 타이머를 제거한다. (타이머 매니저의 것도)
	ClearAllTimerHandle();

	//무적 처리를 하고, Movement를 비활성화
	CharacterStat.bIsInvincibility = true;
	GetCharacterMovement()->Deactivate();
	bUseControllerRotationYaw = false;

	OnCharacterDied.Broadcast();

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
	if (!CharacterState.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return;

	CharacterState.bCanAttack = false; //공격간 Delay,Interval 조절을 위해 세팅
	CharacterState.CharacterActionState = ECharacterActionType::E_Attack;

	int32 nextAnimIdx = 0;
	const float attackSpeed = FMath::Clamp(CharacterStat.DefaultAttackSpeed * WeaponComponent->GetWeaponStat().WeaponAtkSpeedRate, 0.2f, 1.5f);

	//Choose animation which fit battle situation
	TArray <UAnimMontage*> targetAnimList;
	switch (WeaponComponent->GetWeaponStat().WeaponType)
	{
	case EWeaponType::E_Melee:
		//check melee anim type
		if (CharacterState.bIsAirAttacking && GetCharacterMovement()->IsFalling())
		{
			if (AssetHardPtrInfo.AirAttackAnimMontageList.Num() > 0)
			{
				nextAnimIdx = WeaponComponent->GetCurrentComboCnt() % AssetHardPtrInfo.AirAttackAnimMontageList.Num();
				targetAnimList = AssetHardPtrInfo.AirAttackAnimMontageList;

				if (WeaponComponent->GetCurrentComboCnt() == AssetHardPtrInfo.AirAttackAnimMontageList.Num())
				{
					TryDownwardAttack();
					return;
				}
			}
		}
		else if (CharacterState.bIsAirAttacking || GetCharacterMovement()->IsFalling())
		{
			ResetAirAtkLocationUpdateTimer();
			if (IsValid(TargetingManagerComponent->GetTargetedCharacter()))
			{
				TargetingManagerComponent->GetTargetedCharacter()->ResetAirAtkLocationUpdateTimer();
			}
			TryDownwardAttack();
			return;
		}

		else if (AssetHardPtrInfo.MeleeAttackAnimMontageList.Num() > 0)
		{
			nextAnimIdx = WeaponComponent->GetCurrentComboCnt() % AssetHardPtrInfo.MeleeAttackAnimMontageList.Num();
			targetAnimList = AssetHardPtrInfo.MeleeAttackAnimMontageList;
		}
		break;
	case EWeaponType::E_Shoot:
		if (AssetHardPtrInfo.ShootAnimMontageList.Num() > 0)
		{
			nextAnimIdx = WeaponComponent->GetCurrentComboCnt() % AssetHardPtrInfo.ShootAnimMontageList.Num();
		}
		targetAnimList = AssetHardPtrInfo.ShootAnimMontageList;
		break;
	case EWeaponType::E_Throw:
		if (AssetHardPtrInfo.ThrowAnimMontageList.Num() > 0)
		{
			nextAnimIdx = WeaponComponent->GetCurrentComboCnt() % AssetHardPtrInfo.ThrowAnimMontageList.Num();
		}
		targetAnimList = AssetHardPtrInfo.ThrowAnimMontageList;
		break;
	}
	if (targetAnimList.Num() > 0
		&& IsValid(targetAnimList[nextAnimIdx]))
	{
		PlayAnimMontage(targetAnimList[nextAnimIdx], attackSpeed + 0.25f);
	}
}

void ACharacterBase::TryUpperAttack()
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (CharacterState.bIsAirAttacking) return;
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;

	CharacterState.bIsAirAttacking = true;
	WeaponComponent->ResetComboCnt();

	//LaunchCharcter is called by anim notify on upper attack animation

	if (IsValid(AssetHardPtrInfo.UpperAttackAnimMontage))
	{
		PlayAnimMontage(AssetHardPtrInfo.UpperAttackAnimMontage);
	}
}

void ACharacterBase::TryDownwardAttack()
{
	if (!GetCharacterMovement()->IsFalling()) return;

	CharacterState.bIsAirAttacking = false;
	CharacterState.bIsDownwardAttacking = true;
	WeaponComponent->ResetComboCnt();

	if (IsValid(AssetHardPtrInfo.DownwardAttackAnimMontage))
	{
		PlayAnimMontage(AssetHardPtrInfo.DownwardAttackAnimMontage);
	}
}

void ACharacterBase::TryDashAttack()
{
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (CharacterState.bIsAirAttacking || CharacterState.bIsDownwardAttacking) return;
	if (GetCharacterMovement()->IsFalling()) return;
	if (!IsValid(AssetHardPtrInfo.DashAttackAnimMontage)) return;
	if (GetVelocity().Length() <= CharacterState.TotalMoveSpeed * 0.9f) return;

	// Set action state
	CharacterState.CharacterActionState = ECharacterActionType::E_Attack;

	// Activate dash anim with interp to target location
	PlayAnimMontage(AssetHardPtrInfo.DashAttackAnimMontage);
}

void ACharacterBase::DashAttack()
{
	//init local parameter
	const float dashLength = 200.0f;
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
	if (!CharacterState.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return false;

	//스킬 매니저 있는지 확인
	if (!IsValid(SkillManagerComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get SkillmanagerBase"));
		ensure(IsValid(SkillManagerComponent));
		return false;
	}

	//스킬을 보유중인지 확인
	if (!SkillManagerComponent->IsSkillValid(SkillID))
	{
		GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("Skill is not Valid")), FColor::White);
		return false;
	}
	//스킬을 지닐 수 있는 무기와 현재 장비중인 무기가 일치하는지 확인
	ASkillBase* skillBase = SkillManagerComponent->GetOwnSkillBase(SkillID);
	if (skillBase->SkillStat.SkillWeaponStruct.bIsWeaponRequired)
	{
		if (!skillBase->SkillStat.SkillWeaponStruct.AvailableWeaponIDList.Contains(WeaponComponent->WeaponID))
		{
			GamemodeRef.Get()->PrintSystemMessageDelegate.Broadcast(FName(TEXT("Does't have any weapon")), FColor::White);
			return false;
		}
	}

	if(skillBase->SkillState.bIsBlocked) return false;
	else
	{
		CharacterState.bCanAttack = false;
		SkillManagerComponent->TrySkill(SkillID);
		//Reset Combo
		WeaponComponent->ResetComboCnt();
		return true;
	}
}

void ACharacterBase::Attack()
{
	const float attackSpeed = FMath::Min(1.5f, CharacterStat.DefaultAttackSpeed * WeaponComponent->WeaponStat.WeaponAtkSpeedRate);
	WeaponComponent->Attack();
}

void ACharacterBase::StopAttack()
{
	WeaponComponent->StopAttack();
}

void ACharacterBase::TryReload()
{
	if (WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Shoot
		&& WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Throw) return;

	if (WeaponComponent->RangedCombatManager->GetCanReload()) return;

	float reloadTime = WeaponComponent->GetWeaponStat().RangedWeaponStat.LoadingDelayTime;
	if (AssetHardPtrInfo.ReloadAnimMontageList.Num() > 0
		&& IsValid(AssetHardPtrInfo.ReloadAnimMontageList[0]))
	{
		UAnimMontage* reloadAnim = AssetHardPtrInfo.ReloadAnimMontageList[0];
		if (IsValid(reloadAnim))
			PlayAnimMontage(reloadAnim);
	}

	CharacterState.CharacterActionState = ECharacterActionType::E_Reload;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, WeaponComponent->RangedCombatManager, &URangedCombatManager::Reload, reloadTime, false);
}

void ACharacterBase::InitAsset(int32 NewCharacterID)
{
	AssetSoftPtrInfo = GetAssetSoftInfoWithID(NewCharacterID);
	AssetHardPtrInfo = FCharacterAssetHardInfo();
	//SetCharacterAnimAssetInfoData(CharacterID);

	if (!AssetSoftPtrInfo.CharacterMeshSoftPtr.IsNull())
	{
		TArray<FSoftObjectPath> AssetToStream;

		// Mesh 관련
		AssetToStream.AddUnique(AssetSoftPtrInfo.CharacterMeshSoftPtr.ToSoftObjectPath());

		// Animation 관련
		//AssetToStream.AddUnique(AssetSoftPtrInfo.AnimBlueprint.ToSoftObjectPath());

		if (!AssetSoftPtrInfo.MeleeAttackAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.MeleeAttackAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.MeleeAttackAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		AssetToStream.AddUnique(AssetSoftPtrInfo.UpperAttackAnimMontageSoftPtr.ToSoftObjectPath());
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

		if (!AssetSoftPtrInfo.ThrowAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.ThrowAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.ThrowAnimMontageSoftPtrList[i].ToSoftObjectPath());
			}
		}

		if (!AssetSoftPtrInfo.ReloadAnimMontageSoftPtrList.IsEmpty())
		{
			for (int32 i = 0; i < AssetSoftPtrInfo.ReloadAnimMontageSoftPtrList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetSoftPtrInfo.ReloadAnimMontageSoftPtrList[i].ToSoftObjectPath());
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
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.MeleeAttackAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.MeleeAttackAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}

	if (AssetSoftPtrInfo.UpperAttackAnimMontageSoftPtr.IsValid())
	{
		AssetHardPtrInfo.UpperAttackAnimMontage = AssetSoftPtrInfo.UpperAttackAnimMontageSoftPtr.Get();
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
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.ThrowAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.ThrowAnimMontageList.AddUnique(animSoftPtr.Get());
		}
	}
	for (TSoftObjectPtr<UAnimMontage>& animSoftPtr : AssetSoftPtrInfo.ReloadAnimMontageSoftPtrList)
	{
		if (animSoftPtr.IsValid())
		{
			AssetHardPtrInfo.ReloadAnimMontageList.AddUnique(animSoftPtr.Get());
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

void ACharacterBase::InitSoundAsset()
{
	//	TArray<USoundCue*> soundList = SoundAssetInfo.SoundMap.Find("FootStep")->SoundList;
	//	if (!soundList.IsEmpty())
	//	{
	//		FootStepSoundList = soundList;
	//	}	
}

void ACharacterBase::InitMaterialAsset()
{

	for (int8 matIdx = 0; matIdx < AssetHardPtrInfo.DynamicMaterialList.Num(); matIdx++)
	{
		if (IsValid(AssetHardPtrInfo.DynamicMaterialList[matIdx]))
		{
			GetMesh()->SetMaterial(matIdx, GetMesh()->CreateDynamicMaterialInstance(matIdx, AssetHardPtrInfo.DynamicMaterialList[matIdx]));
		}
	}
	/*

	if (!AssetSoftPtrInfo.EmotionTextureList.IsEmpty())
	{
		for (TSoftObjectPtr<UTexture> tex : AssetSoftPtrInfo.EmotionTextureList)
		{
			if (tex.IsValid())
				EmotionTextureList.AddUnique(tex.Get());
		}
	}*/
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

float ACharacterBase::GetTotalStatValue(float& DefaultValue, FStatInfoStruct& AbilityInfo, FStatInfoStruct& SkillInfo, FStatInfoStruct& DebuffInfo)
{
	return DefaultValue
		+ DefaultValue * (AbilityInfo.PercentValue + SkillInfo.PercentValue - DebuffInfo.PercentValue)
		+ (AbilityInfo.FixedValue + SkillInfo.FixedValue - DebuffInfo.FixedValue);
}

bool ACharacterBase::PickWeapon(int32 NewWeaponID)
{
	WeaponComponent->InitWeapon(NewWeaponID);
	return true;
}

void ACharacterBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	GetWorldTimerManager().ClearTimer(KnockDownDelayTimerHandle);
	GetWorldTimerManager().ClearTimer(HitCounterResetTimerHandle);
	GetWorldTimerManager().ClearTimer(LocationInterpHandle);
	GetWorldTimerManager().ClearTimer(AirAtkLocationUpdateHandle);
	AtkIntervalHandle.Invalidate();
	ReloadTimerHandle.Invalidate();
	KnockDownDelayTimerHandle.Invalidate();
	HitCounterResetTimerHandle.Invalidate();
	LocationInterpHandle.Invalidate();
	AirAtkLocationUpdateHandle.Invalidate();
}
