#include "../public/CharacterBase.h"
#include "../../Global/public/DebuffManager.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/RangedWeaponBase.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Global/public/AssetManagerBase.h"
#include "Engine/DamageEvents.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimMontage.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

// Sets default values
ACharacterBase::ACharacterBase()
{	
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->Tags.Add("Character");

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

	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitCharacterState();

	InventoryRef = GetWorld()->SpawnActor<AWeaponInventoryBase>(WeaponInventoryClass, GetActorTransform());
	SubInventoryRef = GetWorld()->SpawnActor<AWeaponInventoryBase>(WeaponInventoryClass, GetActorTransform());
	GamemodeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	if (IsValid(SubInventoryRef) && !SubInventoryRef->IsActorBeingDestroyed())
	{
		SubInventoryRef->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		SubInventoryRef->SetOwner(this);
		SubInventoryRef->InitInventory(4);
	}

	if (IsValid(InventoryRef) && !InventoryRef->IsActorBeingDestroyed())
	{
		InventoryRef->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		InventoryRef->SetOwner(this);
		InventoryRef->InitInventory();
		InitWeaponActors();
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
	//UE_LOG(LogTemp, Warning, TEXT("Debuff State : %d"), NewState.CharacterDebuffState);
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
	if (GetIsActionActive(ECharacterActionType::E_Die)) return 0.0f;

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	const float maxDefenseValue = 2.0f; 
	DamageAmount = FMath::Max(DamageAmount - DamageAmount * (CharacterStat.CharacterDefense / maxDefenseValue), 0.01f);
	if (DamageAmount <= 0.0f || !IsValid(DamageCauser)) return 0.0f;
	if (CharacterStat.bIsInvincibility) return 0.0f;

	CharacterState.CharacterCurrHP = CharacterState.CharacterCurrHP - DamageAmount;
	CharacterState.CharacterCurrHP = FMath::Max(0.0f, CharacterState.CharacterCurrHP);
	if (CharacterState.CharacterCurrHP == 0.0f)
	{
		CharacterState.CharacterActionState = ECharacterActionType::E_Die;
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
	if (GetIsActionActive(ECharacterActionType::E_Die)) return 0.0f;
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

void ACharacterBase::TakeKnockBack(AActor* Causer, float Strength)
{
	if (!IsValid(Causer) || Causer->IsActorBeingDestroyed()) return;
	if (GetIsActionActive(ECharacterActionType::E_Die)) return;

	FVector knockBackDirection = GetActorLocation() - Causer->GetActorLocation();
	knockBackDirection = knockBackDirection.GetSafeNormal();
	knockBackDirection *= Strength;
	knockBackDirection.Z = 0.0f;

	//GetCharacterMovement()->AddImpulse(knockBackDirection);
	LaunchCharacter(knockBackDirection, true, false);
	CharacterState.CharacterActionState = ECharacterActionType::E_Hit;
}

void ACharacterBase::Die()
{
	if (IsValid(InventoryRef) && !InventoryRef->IsActorBeingDestroyed())
	{
		//캐릭터가 죽으면 3가지 타입의 무기 액터를 순차적으로 반환
		for (int weaponIdx = 2; weaponIdx >= 0; weaponIdx--)
			WeaponActorList[weaponIdx]->Destroy();
		InventoryRef->Destroy();
	}
	if (IsValid(GetCurrentWeaponRef()) && !GetCurrentWeaponRef()->IsActorBeingDestroyed())
	{
		GetCurrentWeaponRef()->ClearAllTimerHandle();
	}
	//모든 타이머를 제거한다. (타이머 매니저의 것도)
	ClearAllTimerHandle();
	GamemodeRef->GetGlobalDebuffManagerRef()->ClearAllDebuffTimer(this);

	//무적 처리를 하고, Movement를 비활성화
	CharacterStat.bIsInvincibility = true;
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
	if (!IsValid(GetCurrentWeaponRef())) return;
	if (GetWorldTimerManager().IsTimerActive(AtkIntervalHandle)) return;
	if (!CharacterState.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return;
	
	CharacterState.bCanAttack = false; //공격간 Delay,Interval 조절을 위해 세팅
	CharacterState.CharacterActionState = ECharacterActionType::E_Attack;

	int32 nextAnimIdx = 0;
	const float attackSpeed = FMath::Clamp(CharacterStat.CharacterAtkSpeed * GetCurrentWeaponRef()->GetWeaponStat().WeaponAtkSpeedRate, 0.2f, 1.5f);

	TArray<UAnimMontage*> targetAnimList;
	switch (GetCurrentWeaponRef()->GetWeaponStat().WeaponType)
	{
	case EWeaponType::E_Melee:
		if (AnimAssetData.MeleeAttackAnimMontageList.Num() > 0)
		{
			nextAnimIdx = GetCurrentWeaponRef()->GetCurrentComboCnt() % AnimAssetData.MeleeAttackAnimMontageList.Num();
		}
		targetAnimList = AnimAssetData.MeleeAttackAnimMontageList;
		break;
	case EWeaponType::E_Shoot:
		if (AnimAssetData.ShootAnimMontageList.Num() > 0)
		{
			nextAnimIdx = GetCurrentWeaponRef()->GetCurrentComboCnt() % AnimAssetData.ShootAnimMontageList.Num();
		}
		targetAnimList = AnimAssetData.ShootAnimMontageList;
		break;
	case EWeaponType::E_Throw:
		if (AnimAssetData.ThrowAnimMontageList.Num() > 0)
		{
			nextAnimIdx = GetCurrentWeaponRef()->GetCurrentComboCnt() % AnimAssetData.ThrowAnimMontageList.Num();
		}
		targetAnimList = AnimAssetData.ThrowAnimMontageList;
		break;
	}

	if (targetAnimList.Num() > 0
		&& IsValid(targetAnimList[nextAnimIdx]))
	{
		PlayAnimMontage(targetAnimList[nextAnimIdx], attackSpeed + 0.25f);
	}
}

void ACharacterBase::Attack()
{
	if (!IsValid(GetCurrentWeaponRef()) || GetCurrentWeaponRef()->IsActorBeingDestroyed()) return;
	
	const float attackSpeed = FMath::Min(1.5f, CharacterStat.CharacterAtkSpeed * GetCurrentWeaponRef()->GetWeaponStat().WeaponAtkSpeedRate);

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
	if (AnimAssetData.ReloadAnimMontageList.Num() > 0)
	{
		UAnimMontage* reloadAnim = AnimAssetData.ReloadAnimMontageList[0];
		if (IsValid(reloadAnim))
			PlayAnimMontage(reloadAnim);
	}

	CharacterState.CharacterActionState = ECharacterActionType::E_Reload;
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, FTimerDelegate::CreateLambda([&](){
		Cast<ARangedWeaponBase>(GetCurrentWeaponRef())->TryReload();
		ResetActionState(true);
	}), 1.0f, false, reloadTime);
}

void ACharacterBase::ResetAtkIntervalTimer()
{
	CharacterState.bCanAttack = true;
	GetWorldTimerManager().ClearTimer(AtkIntervalHandle);
}

void ACharacterBase::InitAsset(int32 NewEnemyID)
{
	AssetInfo = GetAssetInfoWithID(NewEnemyID);
	//SetCharacterAnimAssetInfoData(CharacterID);

	if (!AssetInfo.CharacterMesh.IsNull() && !AssetInfo.CharacterMeshMaterial.IsNull() && !AssetInfo.AnimClass.IsNull())
	{
		TArray<FSoftObjectPath> AssetToStream;

		// Mesh 관련
		AssetToStream.AddUnique(AssetInfo.CharacterMesh.ToSoftObjectPath());
		AssetToStream.AddUnique(AssetInfo.CharacterMeshMaterial.ToSoftObjectPath());

		// Animation 관련
		AssetToStream.AddUnique(AssetInfo.AnimClass.ToSoftObjectPath());

		if (!AssetInfo.MeleeAttackAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.MeleeAttackAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.MeleeAttackAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.ShootAnimMontageList.IsEmpty())
		{
			for(int32 i=0;i< AssetInfo.ShootAnimMontageList.Num();i++)
			{
				AssetToStream.AddUnique(AssetInfo.ShootAnimMontageList[i].ToSoftObjectPath());
			}
		}
	
		if (!AssetInfo.ThrowAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.ThrowAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.ThrowAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.ReloadAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.ReloadAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.ReloadAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.HitAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.HitAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.HitAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.RollAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.RollAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.RollAnimMontageList[i].ToSoftObjectPath());
			}
		}
	
		if (!AssetInfo.InvestigateAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.InvestigateAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.InvestigateAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.DieAnimMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.DieAnimMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.DieAnimMontageList[i].ToSoftObjectPath());
			}
		}

		if (!AssetInfo.PointMontageList.IsEmpty())
		{
			for (int32 i = 0; i < AssetInfo.PointMontageList.Num(); i++)
			{
				AssetToStream.AddUnique(AssetInfo.PointMontageList[i].ToSoftObjectPath());
			}
		}

		// Sound

		AssetToStream.AddUnique(AssetInfo.RollSound.ToSoftObjectPath());
		AssetToStream.AddUnique(AssetInfo.BuffSound.ToSoftObjectPath());
		AssetToStream.AddUnique(AssetInfo.DebuffSound.ToSoftObjectPath());
		AssetToStream.AddUnique(AssetInfo.HitImpactSound.ToSoftObjectPath());


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

	/*	if (IsValid(AssetInfo.CharacterMesh.Get()) && IsValid(AssetInfo.CharacterMeshMaterial.Get()) && IsValid(AssetInfo.AnimClass.Get()))
		{
			InitializeAsset();
		}
		else
		{
			FStreamableManager& streamable = UAssetManager::Get().GetStreamableManager();
			streamable.RequestAsyncLoad(AssetToStream, FStreamableDelegate::CreateUObject(this, &ACharacterBase::InitializeAsset));
		}*/
	}
}


void ACharacterBase::SetAsset()
{
	if (AssetInfo.CharacterMesh.IsNull() || !IsValid(AssetInfo.CharacterMesh.Get())) return;
	if (AssetInfo.CharacterMeshMaterial.IsNull() || !IsValid(AssetInfo.CharacterMeshMaterial.Get())) return;
	if (AssetInfo.AnimClass.IsNull() || !IsValid(AssetInfo.AnimClass.Get())) return;

	GetCapsuleComponent()->SetWorldRotation(FRotator::ZeroRotator);
	GetCapsuleComponent()->SetRelativeScale3D(AssetInfo.InitialCapsuleComponentScale);

	GetMesh()->SetSkeletalMesh(AssetInfo.CharacterMesh.Get());
	GetMesh()->SetRelativeLocation(AssetInfo.InitialLocation);
	GetMesh()->SetWorldRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetRelativeScale3D(AssetInfo.InitialScale);

	GetMesh()->SetMaterial(0, AssetInfo.CharacterMeshMaterial.Get());
	GetMesh()->SetAnimInstanceClass(AssetInfo.AnimClass.Get()->GetAnimBlueprintGeneratedClass());
	InitAnimAsset();
	InitSoundAsset();
	InitVFXAsset();
	InitMaterialAsset();
}

bool ACharacterBase::InitAnimAsset()
{
	FCharacterAnimAssetInfoStruct animAssetList;

	if (!AssetInfo.MeleeAttackAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.MeleeAttackAnimMontageList)
		{
			if (anim.IsValid())
				animAssetList.MeleeAttackAnimMontageList.AddUnique(anim.Get());
		}

	}

	if (!AssetInfo.ShootAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.ShootAnimMontageList)
		{
			if (anim.IsValid())
				animAssetList.ShootAnimMontageList.AddUnique(anim.Get());
		}
	}

	if (!AssetInfo.ThrowAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.ThrowAnimMontageList)
		{
			if (anim.IsValid())
				animAssetList.ThrowAnimMontageList.AddUnique(anim.Get());
		}
	}

	if (!AssetInfo.ReloadAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.ReloadAnimMontageList)
		{
			if (anim.IsValid())
				animAssetList.ReloadAnimMontageList.AddUnique(anim.Get());
		}
	}

	if (!AssetInfo.HitAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.HitAnimMontageList)
		{
			if (anim.IsValid())
				animAssetList.HitAnimMontageList.AddUnique(anim.Get());
		}
	}

	if (!AssetInfo.RollAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.RollAnimMontageList)
		{
			if (anim.IsValid())
				animAssetList.RollAnimMontageList.AddUnique(anim.Get());
		}
	}

	if (!AssetInfo.InvestigateAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.InvestigateAnimMontageList)
		{
			if (anim.IsValid())
				animAssetList.InvestigateAnimMontageList.AddUnique(anim.Get());
		}
	}

	if (!AssetInfo.DieAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.DieAnimMontageList)
		{
			if (anim.IsValid())
				animAssetList.DieAnimMontageList.AddUnique(anim.Get());
		}
	}

	if (!AssetInfo.DieAnimMontageList.IsEmpty())
	{
		for (TSoftObjectPtr<UAnimMontage> anim : AssetInfo.PointMontageList)
		{
			if (anim.IsValid())
				PreChaseAnimMontage = anim.Get();
		}
	}

	AnimAssetData = animAssetList;
	return true;
}

void ACharacterBase::InitSoundAsset()
{
	if (AssetInfo.RollSound.IsValid())
	{
		RollSound = AssetInfo.RollSound.Get();
	}

	if (AssetInfo.BuffSound.IsValid())
	{
		BuffSound = AssetInfo.BuffSound.Get();
	}

	if (AssetInfo.HitImpactSound.IsValid())
	{
		HitImpactSound = AssetInfo.HitImpactSound.Get();
	}

	if (AssetInfo.DebuffSound.IsValid())
	{
		DebuffSound = AssetInfo.DebuffSound.Get();
	}
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

FCharacterAssetInfoStruct ACharacterBase::GetAssetInfoWithID(const int32 GetEnemyID)
{
	if (AssetDataInfoTable != nullptr)
	{
		FCharacterAssetInfoStruct* newInfo = nullptr;
		FString rowName = FString::FromInt(GetEnemyID);

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

bool ACharacterBase::EquipWeapon(AWeaponBase* TargetWeapon)
{
	TargetWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, "Weapon_R");
	TargetWeapon->SetActorRelativeLocation(FVector(0.0f), false);
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
}
