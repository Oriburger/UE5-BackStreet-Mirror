#include "../public/CharacterBase.h"
#include "../public/DebuffManagerComponent.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/RangedWeaponBase.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Global/public/AssetManagerBase.h"
#include "../../SkillSystem/public/SkillBase.h"
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
	HitSceneComponent->SetRelativeLocation(FVector(0.0f, 200.0f, 160.0f));


	static ConstructorHelpers::FClassFinder<AWeaponInventoryBase> weaponInventoryClassFinder(TEXT("/Game/Weapon/Blueprint/BP_WeaponInventory"));
	static ConstructorHelpers::FClassFinder<AWeaponBase> meleeWeaponClassFinder(TEXT("/Game/Weapon/Blueprint/BP_MeleeWeaponBase"));
	static ConstructorHelpers::FClassFinder<AWeaponBase> throwWeaponClassFinder(TEXT("/Game/Weapon/Blueprint/BP_ThrowWeaponBase"));
	static ConstructorHelpers::FClassFinder<AWeaponBase> shootWeaponClassFinder(TEXT("/Game/Weapon/Blueprint/BP_ShootWeaponBase"));

	//클래스를 제대로 명시하지 않았으면 크래시를 띄움
	checkf(weaponInventoryClassFinder.Succeeded(), TEXT("Weapon Inventory 클래스 탐색에 실패했습니다."));
	checkf(meleeWeaponClassFinder.Succeeded(), TEXT("Melee Weapon 클래스 탐색에 실패했습니다.")); 
	checkf(shootWeaponClassFinder.Succeeded(), TEXT("Ranged Weapon 클래스 탐색에 실패했습니다."));

	WeaponInventoryClass = weaponInventoryClassFinder.Class; 
	WeaponClassList.Add(meleeWeaponClassFinder.Class);
	WeaponClassList.Add(throwWeaponClassFinder.Class);
	WeaponClassList.Add(shootWeaponClassFinder.Class);

	DebuffManagerComponent = CreateDefaultSubobject<UDebuffManagerComponent>(TEXT("DEBUFF_MANAGER"));

	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CharacterID = AssetInfo.CharacterID;
	InitCharacterState();

	InventoryRef = GetWorld()->SpawnActor<AWeaponInventoryBase>(WeaponInventoryClass, GetActorTransform());
	SubInventoryRef = GetWorld()->SpawnActor<AWeaponInventoryBase>(WeaponInventoryClass, GetActorTransform());
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	
	if (GamemodeRef.IsValid())
	{
		AssetManagerBaseRef = GamemodeRef.Get()->GetGlobalAssetManagerBaseRef();
	}

	if (IsValid(SubInventoryRef) && !SubInventoryRef->IsActorBeingDestroyed())
	{
		SubInventoryRef->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		SubInventoryRef->SetOwner(this);
		SubInventoryRef->InitInventory(2);
	}

	if (IsValid(InventoryRef) && !InventoryRef->IsActorBeingDestroyed())
	{
		InventoryRef->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		InventoryRef->SetOwner(this);
		InventoryRef->InitInventory();
		InitWeaponActors();
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
	GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);
	GetWorld()->GetTimerManager().SetTimer(LocationInterpHandle, updateFunctionDelegate, 0.01f, true);
}

void ACharacterBase::UpdateLocation(const FVector TargetValue, const float InterpSpeed, const bool bAutoReset)
{
	FVector currentLocation = GetActorLocation();
	if (currentLocation.Equals(TargetValue, 50.0f))
	{
		GetWorld()->GetTimerManager().ClearTimer(LocationInterpHandle);
		if (bAutoReset)
		{
			//SetLocationWithInterp(GetActorLocation() , InterpSpeed * 1.5f, false);
		}
	}
	currentLocation = FMath::VInterpTo(currentLocation, TargetValue, 0.1f, InterpSpeed);
	SetActorLocation(currentLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ACharacterBase::SetAirAttackLocation()
{
	if (!GetCharacterMovement()->IsFalling()) return;

	//Set collision query params
	FCollisionQueryParams collisionQueryParam;
	collisionQueryParam.AddIgnoredActor(this);

	//execute linetrace
	FHitResult hitResult;

	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	TEnumAsByte<EObjectTypeQuery> worldStatic = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic);
	TEnumAsByte<EObjectTypeQuery> worldDynamic = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic);
	objectTypes.Add(worldStatic);
	objectTypes.Add(worldDynamic);

	UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), GetActorLocation()
				, GetActorLocation() - FVector(0.0f, 0.0f, 300.0f), objectTypes, false, { this }
				, EDrawDebugTrace::ForDuration, hitResult, true);

	if (hitResult.bBlockingHit)
	{
		LaunchCharacter({ 0.0f, 0.0f, 200.0f }, true, true);
	}
}

void ACharacterBase::OnPlayerLanded(const FHitResult& Hit)
{
	CharacterState.bIsUpperAttacking = false;
}

void ACharacterBase::ResetHitCounter()
{
	CharacterState.HitCounter = 0;
}

void ACharacterBase::KnockDown()
{
	ResetHitCounter();
	//AssetInfo.KnockDownAnimMontageList[0]
	//CharacterState.CharacterActionState = ECharacterActionType::E_KnockedDown;


	if (AssetInfo.AnimationAsset.KnockdownAnimMontageList.IsValidIndex(0)
		&& AssetInfo.AnimationAsset.KnockdownAnimMontageList[0].IsValid())
	{
		PlayAnimMontage(AssetInfo.AnimationAsset.KnockdownAnimMontageList[0].Get());
	}
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
	if(!GamemodeRef.IsValid()) return false;
	
	bool result = DebuffManagerComponent->SetDebuffTimer(DebuffInfo, Causer);
	return result; 
}

bool ACharacterBase::GetDebuffIsActive(ECharacterDebuffType DebuffType)
{
	if(!GamemodeRef.IsValid()) return false;
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
	if (!IsValid(GetCurrentWeaponRef()) || InventoryRef->IsActorBeingDestroyed()) return;
	GetCurrentWeaponRef()->UpdateWeaponStat(NewStat);
}

void ACharacterBase::ResetActionState(bool bForceReset)
{
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Die) return;
	if(!bForceReset && (CharacterState.CharacterActionState == ECharacterActionType::E_Stun
		|| CharacterState.CharacterActionState == ECharacterActionType::E_Reload)) return;
	
	CharacterState.CharacterActionState = ECharacterActionType::E_Idle;
	FWeaponStatStruct currWeaponStat = this->GetCurrentWeaponRef()->GetWeaponStat();
	currWeaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_None;
	this->GetCurrentWeaponRef()->SetWeaponStat(currWeaponStat);
	StopAttack();

	if (!GetWorldTimerManager().IsTimerActive(AtkIntervalHandle))
	{
		CharacterState.bCanAttack = true;
	}
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsValid(DamageCauser)) return 0.0f; 
	if (GetIsActionActive(ECharacterActionType::E_Die)) return 0.0f;

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageAmount <= 0.0f || !IsValid(DamageCauser)) return 0.0f;
	if (CharacterStat.bIsInvincibility) return 0.0f;

	// ====== move to enemy's Hit scene location ===================================
	if (Cast<ACharacterBase>(DamageCauser)->GetCharacterState().bIsUpperAttacking
		&& Cast<ACharacterBase>(DamageCauser)->GetCharacterMovement()->IsFalling())
	{
		float amount = GetCharacterMovement()->IsFalling() ? 300.0f : 1200.0f;
		LaunchCharacter({ 0.0f, 0.0f, amount }, true, true);
	}
	else 
	{
		FVector location = HitSceneComponent->GetComponentLocation();
		FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(DamageCauser->GetActorLocation(), GetActorLocation());
		newRotation.Pitch = newRotation.Roll = 0.0f;
		//if (IsValid(Cast<ACharacterBase>(DamageCauser)->GetCharacterState().TargetedEnemy))
		{
			Cast<ACharacterBase>(DamageCauser)->SetLocationWithInterp(location, 1.0f, false);
			Cast<ACharacterBase>(DamageCauser)->SetActorRotation(newRotation);
		}
	}

	// ====== Hit Counter & Knock Down Check ===========================
	CharacterState.HitCounter += 1;

	// Set hit counter reset event using retriggable timer
	GetWorldTimerManager().ClearTimer(HitCounterResetTimerHandle);
	HitCounterResetTimerHandle.Invalidate();
	GetWorldTimerManager().SetTimer(HitCounterResetTimerHandle, this, &ACharacterBase::ResetHitCounter, 3.0f, false);

	// Check knock down condition and set knock down event using retriggable timer
	if (CharacterState.HitCounter >= 3)
	{
		GetWorldTimerManager().ClearTimer(KnockDownDelayTimerHandle);
		KnockDownDelayTimerHandle.Invalidate();
		GetWorldTimerManager().SetTimer(KnockDownDelayTimerHandle, this, &ACharacterBase::KnockDown, 3.0f, false);
	}
	
	// ======= Damage & Die event ===============================
	CharacterState.CurrentHP = CharacterState.CurrentHP - DamageAmount;
	CharacterState.CurrentHP = FMath::Max(0.0f, CharacterState.CurrentHP);
	if (CharacterState.CurrentHP == 0.0f)
	{
		CharacterState.CharacterActionState = ECharacterActionType::E_Die;
		Die();
	}
	else if (AssetInfo.AnimationAsset.HitAnimMontageList.Num() > 0 && this->CharacterState.CharacterActionState != ECharacterActionType::E_Skill)
	{
		const int32 randomIdx = UKismetMathLibrary::RandomIntegerInRange(0, AssetInfo.AnimationAsset.HitAnimMontageList.Num() - 1);
		if (AssetInfo.AnimationAsset.HitAnimMontageList[randomIdx].IsValid())
		{
			PlayAnimMontage(AssetInfo.AnimationAsset.HitAnimMontageList[randomIdx].Get());
		}
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
	knockBackDirection.Z = 0.0f;

	//GetCharacterMovement()->AddImpulse(knockBackDirection);
	Cast<ACharacterBase>(Target)->LaunchCharacter(knockBackDirection, true, false);
}

void ACharacterBase::Die()
{
	if (IsValid(InventoryRef) && !InventoryRef->IsActorBeingDestroyed())
	{
		//캐릭터가 죽으면 3가지 타입의 무기 액터를 순차적으로 반환
		for (int weaponIdx = 2; weaponIdx >= 0; weaponIdx--)
		{
			for (TPair<int32, ASkillBase*>& skillIdx : WeaponActorList[weaponIdx]->GetWeaponStat().SkillSetInfo.SkillRefMap)
			{
				ASkillBase* skill = skillIdx.Value;
				skill->ClearAllTimerHandle();
				skill->Destroy();
			}
			WeaponActorList[weaponIdx]->GetWeaponStat().SkillSetInfo.SkillRefMap.Empty();
			WeaponActorList[weaponIdx]->Destroy();
			InventoryRef->Destroy();
		}
	}
	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	if (IsValid(GetCurrentWeaponRef()) && !GetCurrentWeaponRef()->IsActorBeingDestroyed())
	{
		GetCurrentWeaponRef()->ClearAllTimerHandle();
	}
	//모든 타이머를 제거한다. (타이머 매니저의 것도)
	ClearAllTimerHandle();
	DebuffManagerComponent->PrintAllDebuff();
	DebuffManagerComponent->ClearDebuffManager();
	DebuffManagerComponent->PrintAllDebuff();

	//무적 처리를 하고, Movement를 비활성화
	CharacterStat.bIsInvincibility = true;
	GetCharacterMovement()->Deactivate();
	bUseControllerRotationYaw = false;

	if (AssetInfo.AnimationAsset.DieAnimMontageList.Num() > 0
		&& AssetInfo.AnimationAsset.DieAnimMontageList[0].IsValid())
	{
		PlayAnimMontage(AssetInfo.AnimationAsset.DieAnimMontageList[0].Get());
	}
	else
	{
		Destroy();
	}
}

void ACharacterBase::TryAttack()
{
	if (!IsValid(GetCurrentWeaponRef())) return;
	
	SetAirAttackLocation();
	
	if (GetWorldTimerManager().IsTimerActive(AtkIntervalHandle)) return;
	if (!CharacterState.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return;
	
	CharacterState.bCanAttack = false; //공격간 Delay,Interval 조절을 위해 세팅
	CharacterState.CharacterActionState = ECharacterActionType::E_Attack;
	

	int32 nextAnimIdx = 0;
	const float attackSpeed = FMath::Clamp(CharacterStat.DefaultAttackSpeed * GetCurrentWeaponRef()->GetWeaponStat().WeaponAtkSpeedRate, 0.2f, 1.5f);

	//Choose animation which fit battle situation
	TArray<TSoftObjectPtr<UAnimMontage>> targetAnimList;
	switch (GetCurrentWeaponRef()->GetWeaponStat().WeaponType)
	{
	case EWeaponType::E_Melee:
		//check melee anim type
		if (!AssetInfo.AnimationAsset.AirAttackAnimMontageList.IsEmpty() && GetCharacterMovement()->IsFalling())
		{
			nextAnimIdx = GetCurrentWeaponRef()->GetCurrentComboCnt() % AssetInfo.AnimationAsset.AirAttackAnimMontageList.Num();
			targetAnimList = AssetInfo.AnimationAsset.AirAttackAnimMontageList;
		}
		else if (AssetInfo.AnimationAsset.MeleeAttackAnimMontageList.Num() > 0)
		{
			targetAnimList = AssetInfo.AnimationAsset.MeleeAttackAnimMontageList;
			nextAnimIdx = GetCurrentWeaponRef()->GetCurrentComboCnt() % AssetInfo.AnimationAsset.MeleeAttackAnimMontageList.Num();
		}
		break;
	case EWeaponType::E_Shoot:
		if (AssetInfo.AnimationAsset.ShootAnimMontageList.Num() > 0)
		{
			nextAnimIdx = GetCurrentWeaponRef()->GetCurrentComboCnt() % AssetInfo.AnimationAsset.ShootAnimMontageList.Num();
		}
		targetAnimList = AssetInfo.AnimationAsset.ShootAnimMontageList;
		break;
	case EWeaponType::E_Throw:
		if (AssetInfo.AnimationAsset.ThrowAnimMontageList.Num() > 0)
		{
			nextAnimIdx = GetCurrentWeaponRef()->GetCurrentComboCnt() % AssetInfo.AnimationAsset.ThrowAnimMontageList.Num();
		}
		targetAnimList = AssetInfo.AnimationAsset.ThrowAnimMontageList;
		break;
	}

	//Play Animation
	if (targetAnimList.Num() > 0
		&& targetAnimList[nextAnimIdx].IsValid())
	{
		PlayAnimMontage(targetAnimList[nextAnimIdx].Get(), attackSpeed + 0.25f);
	}
}

void ACharacterBase::TrySkill()
{
	AWeaponBase* weaponRef = GetCurrentWeaponRef();
	if (!CharacterState.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return;
	if (weaponRef->GetWeaponStat().SkillSetInfo.SkillIDList.IsEmpty())
	{
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("Skill is not binded to this Weapon")), FColor::White);
		return;
	}
	float totalSkillAnimPlayTime = 0;
	int skillAnimIndex = 0 ;
	SkillAnimPlayTimerCurr = 0;

	CharacterState.bCanAttack = false; //공격간 Delay,Interval 조절을 위해 세팅
	CharacterState.CharacterActionState = ECharacterActionType::E_Skill;

	SkillAnimPlayTimerThreshold = AssetInfo.AnimationAsset.SkillAnimMontageMap.Find(weaponRef->WeaponID)->SkillAnimMontageList.Num();
	
	//Total skill animation play time which is using for init skill timing.
	for (auto& skillAnimMontage : AssetInfo.AnimationAsset.SkillAnimMontageMap.Find(weaponRef->WeaponID)->SkillAnimMontageList) 
	{
		totalSkillAnimPlayTime += skillAnimMontage->CalculateSequenceLength()/GetSkillAnimPlayRate(skillAnimIndex);
		GetCurrentWeaponRef()->WeaponStat.SkillSetInfo.TotalSkillPlayTime = totalSkillAnimPlayTime;
		skillAnimIndex++;
	}

	SkillAnimPlayTimerHandleList.SetNum(SkillAnimPlayTimerThreshold);
	PlaySkillAnimation();
}
float ACharacterBase::GetSkillAnimPlayRate(uint8 SkillAnimIndex)
{
	float animPlayRate = 0.0f;

	if (GetCurrentWeaponRef()->WeaponStat.SkillSetInfo.IsAnimPlayRateSyncWithGrade)
	{
		if (UKismetMathLibrary::InRange_FloatFloat(CharacterState.CharacterCurrSkillGauge, GetCurrentWeaponRef()->WeaponStat.SkillGaugeInfo.SkillCommonReq, GetCurrentWeaponRef()->WeaponStat.SkillGaugeInfo.SkillRareReq, true, false))
		{
			animPlayRate = GetCurrentWeaponRef()->WeaponStat.SkillSetInfo.CommonSkillAnimRateList[SkillAnimPlayTimerCurr];
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(CharacterState.CharacterCurrSkillGauge, GetCurrentWeaponRef()->WeaponStat.SkillGaugeInfo.SkillRareReq, GetCurrentWeaponRef()->WeaponStat.SkillGaugeInfo.SkillLegendReq, true, false))
		{
			animPlayRate = GetCurrentWeaponRef()->WeaponStat.SkillSetInfo.RareSkillAnimRateList[SkillAnimPlayTimerCurr];
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(CharacterState.CharacterCurrSkillGauge, GetCurrentWeaponRef()->WeaponStat.SkillGaugeInfo.SkillLegendReq, GetCurrentWeaponRef()->WeaponStat.SkillGaugeInfo.SkillMythicReq, true, false))
		{
			animPlayRate = GetCurrentWeaponRef()->WeaponStat.SkillSetInfo.LegendSkillAnimRateList[SkillAnimPlayTimerCurr];
		}
		else if (CharacterState.CharacterCurrSkillGauge >= GetCurrentWeaponRef()->WeaponStat.SkillGaugeInfo.SkillMythicReq)
		{
			animPlayRate = GetCurrentWeaponRef()->WeaponStat.SkillSetInfo.MythicSkillAnimRateList[SkillAnimPlayTimerCurr];
		}
	}
	else
	{
		animPlayRate = GetCurrentWeaponRef()->WeaponStat.SkillSetInfo.CommonSkillAnimRateList[SkillAnimPlayTimerCurr];
	}
	return animPlayRate;
}

void ACharacterBase::PlaySkillAnimation()
{

	if (SkillAnimPlayTimerCurr >= SkillAnimPlayTimerThreshold)
	{
		SkillAnimPlayTimerHandleList.Empty();
		return;
	}
	float animPlayRate = GetSkillAnimPlayRate(SkillAnimPlayTimerCurr);
	TArray<TSoftObjectPtr<UAnimMontage>> targetAnimList = AssetInfo.AnimationAsset.SkillAnimMontageMap.Find(GetCurrentWeaponRef()->WeaponID)->SkillAnimMontageList;
	
	if (targetAnimList[SkillAnimPlayTimerCurr].IsValid())
	{
		float animPlayTime = PlayAnimMontage(targetAnimList[SkillAnimPlayTimerCurr].Get(), animPlayRate);
		GetWorldTimerManager().SetTimer(SkillAnimPlayTimerHandleList[SkillAnimPlayTimerCurr], this, &ACharacterBase::PlayNextSkillAnimation, animPlayTime, false);
	}
}

void ACharacterBase::PlayNextSkillAnimation()
{
	SkillAnimPlayTimerCurr++;
	PlaySkillAnimation();
}

void ACharacterBase::Attack()
{
	if (!IsValid(GetCurrentWeaponRef()) || GetCurrentWeaponRef()->IsActorBeingDestroyed()) return;
	
	const float attackSpeed = FMath::Min(1.5f, CharacterStat.DefaultAttackSpeed * GetCurrentWeaponRef()->GetWeaponStat().WeaponAtkSpeedRate);

	GetCurrentWeaponRef()->Attack();
}
 
void ACharacterBase::StopAttack()
{
	if (!IsValid(GetCurrentWeaponRef()) || GetCurrentWeaponRef()->IsActorBeingDestroyed()) return;
	GetCurrentWeaponRef()->StopAttack();
}

void ACharacterBase::TryReload()
{
	if (!IsValid(GetCurrentWeaponRef()) || GetCurrentWeaponRef()->IsActorBeingDestroyed()) return;

	if (GetCurrentWeaponRef()->GetWeaponStat().WeaponType != EWeaponType::E_Shoot
		&& GetCurrentWeaponRef()->GetWeaponStat().WeaponType != EWeaponType::E_Throw) return; 
	
	if (!Cast<ARangedWeaponBase>(GetCurrentWeaponRef())->GetCanReload()) return;

	float reloadTime = GetCurrentWeaponRef()->GetWeaponStat().RangedWeaponStat.LoadingDelayTime;
	if (AssetInfo.AnimationAsset.ReloadAnimMontageList.Num() > 0
		&& AssetInfo.AnimationAsset.ReloadAnimMontageList[0].IsValid())
	{
		UAnimMontage* reloadAnim = AssetInfo.AnimationAsset.ReloadAnimMontageList[0].Get();
		if (IsValid(reloadAnim))
			PlayAnimMontage(reloadAnim);
	}

	CharacterState.CharacterActionState = ECharacterActionType::E_Reload;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, Cast<ARangedWeaponBase>(GetCurrentWeaponRef()), &ARangedWeaponBase::Reload, reloadTime, false);
}

void ACharacterBase::InitAsset(int32 NewEnemyID)
{
	AssetInfo = GetAssetInfoWithID(NewEnemyID);
	//SetCharacterAnimAssetInfoData(CharacterID);

	if (!AssetInfo.CharacterMesh.IsNull() && AssetInfo.CharacterMeshMaterialList.Num() != 0)
	{
		TArray<FSoftObjectPath> AssetToStream;

		// Mesh 관련
		AssetToStream.AddUnique(AssetInfo.CharacterMesh.ToSoftObjectPath());
		for (int32 idx = 0; idx < AssetInfo.CharacterMeshMaterialList.Num(); idx++)
		{
			if (AssetInfo.CharacterMeshMaterialList.IsValidIndex(idx))
			{
				AssetToStream.AddUnique(AssetInfo.CharacterMeshMaterialList[idx].ToSoftObjectPath());
			}
		}

		// Animation 관련
		//AssetToStream.AddUnique(AssetInfo.AnimationAsset.AnimBlueprint.ToSoftObjectPath());

		if (!AssetInfo.AnimationAsset.MeleeAttackAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.MeleeAttackAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.MeleeAttackAnimMontageList[i].ToSoftObjectPath());
			}
		}

		AssetToStream.AddUnique(AssetInfo.AnimationAsset.UpperAttackAminMontage.ToSoftObjectPath());
		if (!AssetInfo.AnimationAsset.AirAttackAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.AirAttackAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.AirAttackAnimMontageList[i].ToSoftObjectPath());
			}
		}


		if (!AssetInfo.AnimationAsset.ShootAnimMontageList.IsEmpty())
		{
			for(int32 i=0;i< AssetInfo.AnimationAsset.ShootAnimMontageList.Num();i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.ShootAnimMontageList[i].ToSoftObjectPath());
			}
		}
	
		if (!AssetInfo.AnimationAsset.ThrowAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.ThrowAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.ThrowAnimMontageList[i].ToSoftObjectPath());
			}	
		}

		if (!AssetInfo.AnimationAsset.ReloadAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.ReloadAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.ReloadAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.AnimationAsset.HitAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.HitAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.HitAnimMontageList[i].ToSoftObjectPath());
			}
		}

		for (auto index = AssetInfo.AnimationAsset.SkillAnimMontageMap.CreateConstIterator(); index; ++index)
		{
			TArray<TSoftObjectPtr<UAnimMontage>> skillAnimMontageList = index.Value().SkillAnimMontageList;
			for (auto& anim : skillAnimMontageList)
			{
				if (!skillAnimMontageList.IsEmpty())
				{
					AssetToStream.AddUnique(anim.ToSoftObjectPath());
				}
			}
		}

		if (!AssetInfo.AnimationAsset.RollAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.RollAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.RollAnimMontageList[i].ToSoftObjectPath());
			}
		}
	
		if (!AssetInfo.AnimationAsset.InvestigateAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.InvestigateAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.InvestigateAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.AnimationAsset.DieAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.DieAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.DieAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.AnimationAsset.PointMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.PointMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.PointMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.AnimationAsset.KnockdownAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.AnimationAsset.KnockdownAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.AnimationAsset.KnockdownAnimMontageList[i].ToSoftObjectPath());
			}
		}

		// VFX
		if (!AssetInfo.DebuffNiagaraEffectList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.DebuffNiagaraEffectList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.DebuffNiagaraEffectList[i].ToSoftObjectPath());
			}
		}

		// material
		AssetToStream.AddUnique(AssetInfo.NormalMaterial.ToSoftObjectPath());
		AssetToStream.AddUnique(AssetInfo.WallThroughMaterial.ToSoftObjectPath());
		if (!AssetInfo.EmotionTextureList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.EmotionTextureList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.EmotionTextureList[i].ToSoftObjectPath());
			}
		}

		FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
		streamable.RequestAsyncLoad(AssetToStream, FStreamableDelegate::CreateUObject(this, &ACharacterBase::SetAsset));

	}
}


void ACharacterBase::SetAsset()
{
	if (AssetInfo.CharacterMesh.IsNull() || !IsValid(AssetInfo.CharacterMesh.Get())) return;
	if (AssetInfo.CharacterMeshMaterialList.Num() == 0) return;
	//if (AssetInfo.AnimBlueprint.IsNull() || !IsValid(AssetInfo.AnimBlueprint.Get())) return;

	GetCapsuleComponent()->SetWorldRotation(FRotator::ZeroRotator);
	SetActorScale3D(AssetInfo.InitialScale);

	GetMesh()->SetSkeletalMesh(AssetInfo.CharacterMesh.Get());
	GetMesh()->SetRelativeLocation(AssetInfo.InitialLocation);
	GetMesh()->SetWorldRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeScale3D(FVector(1.0f));

	const int32 meshMatCount = AssetInfo.CharacterMeshMaterialList.Num();
	for (int matIdx = 0; matIdx < GetMesh()->GetMaterials().Num(); matIdx++)
	{
		UMaterialInterface* targetMat = AssetInfo.CharacterMeshMaterialList[matIdx % meshMatCount].Get();
		GetMesh()->SetMaterial(matIdx, targetMat);
	}
	GetMesh()->SetAnimInstanceClass(AssetInfo.AnimationAsset.AnimBlueprint);
	InitAnimAsset();
	InitSoundAsset();
	InitVFXAsset();
	InitMaterialAsset();
}

bool ACharacterBase::InitAnimAsset()
{

	return true;
}

void ACharacterBase::InitVFXAsset()
{
	if (!AssetInfo.DebuffNiagaraEffectList.IsEmpty())
	{
		for (TSoftObjectPtr<UNiagaraSystem> vfx : AssetInfo.DebuffNiagaraEffectList)
		{
			if (vfx.IsValid())
				DebuffNiagaraEffectList.AddUnique(vfx.Get());
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
	if (AssetInfo.NormalMaterial.IsValid())
	{
		NormalMaterial = AssetInfo.NormalMaterial.Get();
	}

	if (AssetInfo.WallThroughMaterial.IsValid())
	{
		WallThroughMaterial = AssetInfo.WallThroughMaterial.Get();
	}

	if (!AssetInfo.EmotionTextureList.IsEmpty())
	{
		for (TSoftObjectPtr<UTexture> tex : AssetInfo.EmotionTextureList)
		{
			if (tex.IsValid())
				EmotionTextureList.AddUnique(tex.Get());
		}
	}
}

FCharacterAssetInfoStruct ACharacterBase::GetAssetInfoWithID(const int32 TargetCharacterID)
{
	if (AssetDataInfoTable != nullptr)
	{
		FCharacterAssetInfoStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(TargetCharacterID);

		newInfo = AssetDataInfoTable->FindRow<FCharacterAssetInfoStruct>(FName(rowName), rowName);

		if (newInfo != nullptr) return *newInfo;
	}
	return FCharacterAssetInfoStruct();
}

void ACharacterBase::InitDynamicMeshMaterial(UMaterialInterface* NewMaterial)
{
	if (NewMaterial == nullptr) return;

	for (int8 matIdx = 0; matIdx < GetMesh()->GetNumMaterials(); matIdx += 1)
	{
		CurrentDynamicMaterial = GetMesh()->CreateDynamicMaterialInstance(matIdx, NewMaterial);
	}
}

float ACharacterBase::GetTotalStatValue(float& DefaultValue, FStatInfoStruct& AbilityInfo, FStatInfoStruct& SkillInfo, FStatInfoStruct& DebuffInfo)
{
	return DefaultValue 
			+ DefaultValue * (AbilityInfo.PercentValue + SkillInfo.PercentValue - DebuffInfo.PercentValue)
			+ (AbilityInfo.FixedValue + SkillInfo.FixedValue - DebuffInfo.FixedValue); 
}

bool ACharacterBase::EquipWeapon(AWeaponBase* TargetWeapon)
{
	TargetWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, "Weapon_R");
	TargetWeapon->SetActorRelativeLocation(FVector(0.0f), false);
	TargetWeapon->SetActorRelativeRotation(FRotator::ZeroRotator, false);
	TargetWeapon->SetOwnerCharacter(this);
	CurrentWeaponRef = TargetWeapon;
	return true;
}

bool ACharacterBase::PickWeapon(int32 NewWeaponID)
{
	if (!IsValid(InventoryRef)) return false;
	bool result = InventoryRef->AddWeapon(NewWeaponID);
	return result;
}

void ACharacterBase::SwitchToNextWeapon()
{
	if (!IsValid(InventoryRef)) return;
	if (!IsValid(GetCurrentWeaponRef()) || GetCurrentWeaponRef()->IsActorBeingDestroyed()) return;
	InventoryRef->SwitchToNextWeapon();
}

void ACharacterBase::DropWeapon()
{
	if (!IsValid(GetCurrentWeaponRef())) return;
	
	if (CurrentWeaponRef->GetWeaponType() == EWeaponType::E_Throw)
	{
		SubInventoryRef->RemoveCurrentWeapon();
		SwitchWeaponActor(EWeaponType::E_Melee);
		InventoryRef->EquipWeaponByIdx(0);
	}
	else InventoryRef->RemoveCurrentWeapon();
	
	SwitchToNextWeapon();
		
	/*---- 현재 무기를 월드에 버리는 기능은 미구현 -----*/
}

bool ACharacterBase::TrySwitchToSubWeapon(int32 SubWeaponIdx)
{
	if (SubWeaponIdx >= SubInventoryRef->GetCurrentWeaponCount()) return false;

	if (GetCurrentWeaponRef()->GetWeaponType() == EWeaponType::E_Throw)
		SubInventoryRef->SyncCurrentWeaponInfo(true); //기존 무기 정보를 저장
	else
		InventoryRef->SyncCurrentWeaponInfo(true); //기존 무기 정보를 저장
	SubInventoryRef->EquipWeaponByIdx(SubWeaponIdx);

	return true;
}

AWeaponInventoryBase* ACharacterBase::GetInventoryRef()
{
	if (!IsValid(InventoryRef) || InventoryRef->IsActorBeingDestroyed()) return nullptr;
	return InventoryRef;
}

AWeaponInventoryBase* ACharacterBase::GetSubInventoryRef()
{	
	if (!IsValid(SubInventoryRef) || SubInventoryRef->IsActorBeingDestroyed()) return nullptr;
	return SubInventoryRef;
}

AWeaponBase* ACharacterBase::GetCurrentWeaponRef()
{
	if (!CurrentWeaponRef.IsValid()) return nullptr;
	return CurrentWeaponRef.Get();
}

void ACharacterBase::SwitchWeaponActor(EWeaponType TargetWeaponType)
{
	if (!WeaponActorList.IsValidIndex((int32)TargetWeaponType-1)) return;
	if (!IsValid(WeaponActorList[(int32)TargetWeaponType-1]))  return;
	if (!IsValid(GetCurrentWeaponRef())) return;

	GetCurrentWeaponRef()->SetActorHiddenInGame(true);
	GetCurrentWeaponRef()->InitWeapon(0);
	CurrentWeaponRef = WeaponActorList[(int32)TargetWeaponType - 1];
	GetCurrentWeaponRef()->SetActorHiddenInGame(false);
}

AWeaponBase* ACharacterBase::SpawnWeaponActor(EWeaponType TargetWeaponType)
{
	checkf(WeaponClassList.Num() == 3, TEXT("초기 무기 클래스 지정이 실패했습니다."));
	TSubclassOf<AWeaponBase> weaponClass = WeaponClassList[(int)TargetWeaponType-1];

	if (!IsValid(weaponClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon Class가 Invalid 합니다."));
		return nullptr;
	}
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector spawnLocation = GetActorLocation();
	const FRotator spawnRotation = FRotator();
	const FVector spawnScale3D = ActorHasTag("Player") ? FVector(2.0f) : FVector(1.0f / GetCapsuleComponent()->GetComponentScale().X);
	FTransform spawnTransform = FTransform(spawnRotation, spawnLocation, spawnScale3D);
	AWeaponBase* newWeapon = Cast<AWeaponBase>(GetWorld()->SpawnActor(weaponClass, &spawnTransform, spawnParams));
	newWeapon->InitWeapon(0);

	return newWeapon;
}

void ACharacterBase::InitWeaponActors()
{
	if (!WeaponActorList.IsEmpty()) return; 

	//근접과 원거리의 무기 액터를 스폰
	for (int idx = 0; idx < 3; idx++)
	{
		WeaponActorList.Add(SpawnWeaponActor((EWeaponType)(idx+1)));
		WeaponActorList[idx]->SetOwner(this);
		checkf(IsValid(WeaponActorList[idx]), TEXT("무기 %d 타입 스폰에 실패했습니다."));
		EquipWeapon(WeaponActorList[idx]);
	} 
	EquipWeapon(WeaponActorList[0]);
	CurrentWeaponRef = WeaponActorList[0];
}

void ACharacterBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	AtkIntervalHandle.Invalidate();
	ReloadTimerHandle.Invalidate();

	for (int num=0 ; num < SkillAnimPlayTimerHandleList.Num(); num++)
	{
		GetWorldTimerManager().ClearTimer(SkillAnimPlayTimerHandleList[num]);
		SkillAnimPlayTimerHandleList[num].Invalidate();
	}
	SkillAnimPlayTimerHandleList.Empty();
}
