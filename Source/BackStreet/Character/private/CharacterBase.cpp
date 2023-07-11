#include "../public/CharacterBase.h"
#include "../../Global/public/DebuffManager.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/RangedWeaponBase.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Global/public/AssetManagerBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"

// Sets default values
ACharacterBase::ACharacterBase()
{	
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->Tags.Add("Character");

	InventoryComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("INVENTORY"));
	InventoryComponent->SetupAttachment(GetCapsuleComponent());
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitCharacterState();

	InventoryRef = Cast<AWeaponInventoryBase>(InventoryComponent->GetChildActor());
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	if (IsValid(InventoryRef) && !InventoryRef->IsActorBeingDestroyed())
	{
		InventoryRef->SetOwner(this);
		InventoryRef->InitInventory();
	}

	//애니메이션 에셋 초기화
	if (GamemodeRef.IsValid())
	{
		AnimAssetData = GamemodeRef.Get()->GetCharacterAnimAssetInfoData(CharacterStat.CharacterID);
	}
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

void ACharacterBase::InitCharacterState()
{
	GetCharacterMovement()->MaxWalkSpeed = CharacterStat.CharacterMoveSpeed;
	CharacterState.CharacterCurrHP = CharacterStat.CharacterMaxHP;
	CharacterState.bCanAttack = true;
	CharacterState.CharacterActionState = ECharacterActionType::E_Idle;
}

bool ACharacterBase::TryAddNewDebuff(ECharacterDebuffType NewDebuffType, AActor* Causer, float TotalTime, float Value)
{
	if(!GamemodeRef.IsValid()) return false;
	
	if (!IsValid(GamemodeRef.Get()->GetGlobalDebuffManagerRef())) return false;

	bool result = GamemodeRef.Get()->GetGlobalDebuffManagerRef()->SetDebuffTimer(NewDebuffType, this, Causer, TotalTime, Value);
	return result; 
}

bool ACharacterBase::GetDebuffIsActive(ECharacterDebuffType DebuffType)
{
	if(!GamemodeRef.IsValid()) return false;
	if (!IsValid(GamemodeRef.Get()->GetGlobalDebuffManagerRef())) return false;
	return	GamemodeRef.Get()->GetGlobalDebuffManagerRef()->GetDebuffIsActive(DebuffType, this);
}

void ACharacterBase::UpdateCharacterStat(FCharacterStatStruct NewStat)
{
	CharacterStat = NewStat;
	GetCharacterMovement()->MaxWalkSpeed = CharacterStat.CharacterMoveSpeed;
}

void ACharacterBase::UpdateCharacterState(FCharacterStateStruct NewState)
{
	UE_LOG(LogTemp, Warning, TEXT("Debuff State : %d"), NewState.CharacterDebuffState);
	CharacterState = NewState;
}

void ACharacterBase::UpdateWeaponStat(FWeaponStatStruct NewStat)
{
	if (!IsValid(WeaponRef) || InventoryRef->IsActorBeingDestroyed()) return;
	WeaponRef->UpdateWeaponStat(NewStat);
}

void ACharacterBase::ResetActionState(bool bForceReset)
{
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Die) return;
	if(!bForceReset && (CharacterState.CharacterActionState == ECharacterActionType::E_Stun
		|| CharacterState.CharacterActionState == ECharacterActionType::E_Reload)) return;

	CharacterState.CharacterActionState = ECharacterActionType::E_Idle;
	StopAttack();

	if (!GetWorldTimerManager().IsTimerActive(AtkIntervalHandle))
	{
		CharacterState.bCanAttack = true;
	}
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator
								, AActor* DamageCauser)
{
	if (!IsValid(DamageCauser)) return 0.0f; 

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	DamageAmount = DamageAmount - DamageAmount * CharacterStat.CharacterDefense;
	if (DamageAmount <= 0.0f || !IsValid(DamageCauser)) return 0.0f;
	if (CharacterStat.bIsInvincibility) return 0.0f;

	CharacterState.CharacterCurrHP = CharacterState.CharacterCurrHP - DamageAmount;
	CharacterState.CharacterCurrHP = FMath::Max(0.0f, CharacterState.CharacterCurrHP);
	if (CharacterState.CharacterCurrHP == 0.0f)
	{
		Die();
	}
	else if (AnimAssetData.HitAnimMontageList.Num() > 0)
	{
		const int32 randomIdx = UKismetMathLibrary::RandomIntegerInRange(0, AnimAssetData.HitAnimMontageList.Num() - 1);
		PlayAnimMontage(AnimAssetData.HitAnimMontageList[randomIdx]);
	}
	return DamageAmount;
}

float ACharacterBase::TakeDebuffDamage(float DamageAmount, ECharacterDebuffType DebuffType, AActor* Causer)
{
	if (!IsValid(Causer)) return 0.0f;
	TakeDamage(DamageAmount, FDamageEvent(), nullptr, Causer);
	return DamageAmount;
}

void ACharacterBase::TakeHeal(float HealAmountRate, bool bIsTimerEvent, uint8 BuffDebuffType)
{
	CharacterState.CharacterCurrHP += CharacterStat.CharacterMaxHP * HealAmountRate;
	CharacterState.CharacterCurrHP = FMath::Min(CharacterStat.CharacterMaxHP, CharacterState.CharacterCurrHP);
	return;
}

void ACharacterBase::Die()
{
	if (IsValid(InventoryRef) && !InventoryRef->IsActorBeingDestroyed())
	{
		InventoryRef->RemoveCurrentWeapon();
		InventoryRef->Destroy();
	}
	if (IsValid(WeaponRef) && !WeaponRef->IsActorBeingDestroyed())
	{
		WeaponRef->ClearAllTimerHandle();
	}

	ClearAllTimerHandle();
	CharacterState.CharacterActionState = ECharacterActionType::E_Die;
	CharacterStat.bIsInvincibility = true;
	ClearAllTimerHandle();
	GetCharacterMovement()->Deactivate();
	bUseControllerRotationYaw = false;

	if (AnimAssetData.DieAnimMontageList.Num() > 0)
	{
		PlayAnimMontage(AnimAssetData.DieAnimMontageList[0]);
	}
	else
	{
		Destroy();
	}
}

void ACharacterBase::TryAttack()
{
	if (!IsValid(WeaponRef)) return;
	if (GetWorldTimerManager().IsTimerActive(AtkIntervalHandle)) return;
	if (!CharacterState.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return;
	
	CharacterState.bCanAttack = false; //공격간 Delay,Interval 조절을 위해 세팅
	CharacterState.CharacterActionState = ECharacterActionType::E_Attack;

	int32 nextAnimIdx = 0;
	const float attackSpeed = FMath::Min(1.5f, CharacterStat.CharacterAtkSpeed * WeaponRef->GetWeaponStat().WeaponAtkSpeedRate);

	TArray<UAnimMontage*> targetAnimList;
	switch (WeaponRef->GetWeaponStat().WeaponType)
	{
	case EWeaponType::E_Melee:
		if (AnimAssetData.MeleeAttackAnimMontageList.Num() > 0)
		{
			nextAnimIdx = WeaponRef->GetCurrentComboCnt() % AnimAssetData.MeleeAttackAnimMontageList.Num();
		}
		targetAnimList = AnimAssetData.MeleeAttackAnimMontageList;
		break;
	case EWeaponType::E_Shoot:
		if (AnimAssetData.ShootAnimMontageList.Num() > 0)
		{
			nextAnimIdx = WeaponRef->GetCurrentComboCnt() % AnimAssetData.ShootAnimMontageList.Num();
		}
		targetAnimList = AnimAssetData.ShootAnimMontageList;
		break;
	case EWeaponType::E_Throw:
		if (AnimAssetData.ThrowAnimMontageList.Num() > 0)
		{
			nextAnimIdx = WeaponRef->GetCurrentComboCnt() % AnimAssetData.ThrowAnimMontageList.Num();
		}
		targetAnimList = AnimAssetData.ThrowAnimMontageList;
		break;
	}

	if (targetAnimList.Num() > 0
		&& IsValid(targetAnimList[nextAnimIdx]))
	{
		PlayAnimMontage(targetAnimList[nextAnimIdx], attackSpeed + 0.75f);
	}
}

void ACharacterBase::Attack()
{
	if (!IsValid(WeaponRef) || WeaponRef->IsActorBeingDestroyed()) return;
	
	const float attackSpeed = FMath::Min(1.5f, CharacterStat.CharacterAtkSpeed * WeaponRef->GetWeaponStat().WeaponAtkSpeedRate);

	WeaponRef->Attack();
}
 
void ACharacterBase::StopAttack()
{
	if (!IsValid(WeaponRef) || WeaponRef->IsActorBeingDestroyed()) return;
	WeaponRef->StopAttack();
}

void ACharacterBase::TryReload()
{
	if (!IsValid(WeaponRef) || WeaponRef->IsActorBeingDestroyed()) return;
	if (WeaponRef->GetWeaponStat().WeaponType != EWeaponType::E_Shoot
		&& WeaponRef->GetWeaponStat().WeaponType != EWeaponType::E_Throw)	return; 
	if (!Cast<ARangedWeaponBase>(WeaponRef)->GetCanReload()) return;

	float reloadTime = WeaponRef->GetWeaponStat().RangedWeaponStat.LoadingDelayTime;
	if (AnimAssetData.ReloadAnimMontageList.Num() > 0)
	{
		UAnimMontage* reloadAnim = AnimAssetData.ReloadAnimMontageList[0];
		if (IsValid(reloadAnim))
			PlayAnimMontage(reloadAnim);
	}

	CharacterState.CharacterActionState = ECharacterActionType::E_Reload;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, FTimerDelegate::CreateLambda([&](){
		Cast<ARangedWeaponBase>(WeaponRef)->TryReload();
		ResetActionState(true);
	}), 1.0f, false, reloadTime);
}

void ACharacterBase::ResetAtkIntervalTimer()
{
	CharacterState.bCanAttack = true;
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
}

void ACharacterBase::InitDynamicMeshMaterial(UMaterialInterface* NewMaterial)
{
	if (NewMaterial == nullptr) return;

	for (int8 matIdx = 0; matIdx < GetMesh()->GetNumMaterials(); matIdx += 1)
	{
		CurrentDynamicMaterial = GetMesh()->CreateDynamicMaterialInstance(matIdx, NewMaterial);
	}
}

bool ACharacterBase::EquipWeapon(AWeaponBase* TargetWeapon)
{
	TargetWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, "Weapon_R");
	TargetWeapon->SetActorRelativeLocation(FVector(0.0f), false);
	TargetWeapon->SetOwnerCharacter(this);
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
	if (!IsValid(WeaponRef) || WeaponRef->IsActorBeingDestroyed()) return;
	InventoryRef->SwitchToNextWeapon();
}

void ACharacterBase::DropWeapon()
{
	if (!IsValid(InventoryRef)) return;

	InventoryRef->RemoveCurrentWeapon();
	/*---- 현재 무기를 월드에 버리는 기능은 미구현 -----*/
}

AWeaponInventoryBase* ACharacterBase::GetInventoryRef()
{
	if (!IsValid(InventoryRef) || InventoryRef->IsActorBeingDestroyed()) return nullptr;
	return InventoryRef;
}

void ACharacterBase::SetWeaponActorRef(AWeaponBase* NewWeapon)
{
	if (!IsValid(NewWeapon) || NewWeapon->IsActorBeingDestroyed()) return;
	WeaponRef = NewWeapon;
}

AWeaponBase* ACharacterBase::GetWeaponActorRef()
{
	if (!IsValid(WeaponRef) || WeaponRef->IsActorBeingDestroyed()) return nullptr;

	return WeaponRef;
}

void ACharacterBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
}
