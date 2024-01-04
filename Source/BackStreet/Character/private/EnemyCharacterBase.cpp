// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/EnemyCharacterBase.h"
#include "../../AISystem/public/AIControllerBase.h"
#include "../public/CharacterInfoStruct.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/ItemBase.h"
#include "../../StageSystem/public/StageInfoStruct.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Global/public/SkillManagerBase.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../public/MainCharacterBase.h"

#define TURN_TIME_OUT_SEC 1.0f

AEnemyCharacterBase::AEnemyCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	FloatingHpBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("FLOATING_HP_BAR"));
	FloatingHpBar->SetupAttachment(GetCapsuleComponent());
	FloatingHpBar->SetRelativeLocation(FVector(0.0f, 0.0f, 85.0f));
	FloatingHpBar->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
	FloatingHpBar->SetDrawSize({ 80.0f, 10.0f });

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	this->Tags.Add("Enemy");
	// 난이도 조정용 Tag
	this->Tags.Add("Easy");
}

void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultController();

	InitFloatingHpWidget();
	InitEnemyStat();
	
	SetDefaultWeapon();
	InitDynamicMeshMaterial(GetMesh()->GetMaterial(0));
}

void AEnemyCharacterBase::InitEnemyStat()
{
	UE_LOG(LogTemp, Warning, TEXT("My ID : %d"), CharacterID);
	GamemodeRef->UpdateCharacterStatWithID(this, CharacterID);
	CharacterState.CharacterCurrHP = CharacterStat.CharacterMaxHP;
	GetCharacterMovement()->MaxWalkSpeed = CharacterStat.CharacterMoveSpeed;
	SetDefaultStat();
}

float AEnemyCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!IsValid(DamageCauser) || !DamageCauser->ActorHasTag("Player") || damageAmount <= 0.0f) return 0.0f;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitImpactSound, GetActorLocation());
	EnemyDamageDelegate.ExecuteIfBound(DamageCauser);

	//const float knockBackStrength = 100000.0f;


	if (DamageCauser->ActorHasTag("Player"))
	{
		if (DamageCauser->ActorHasTag("Attack|Common"))
		{
			Cast<AMainCharacterBase>(DamageCauser)->AddSkillGauge();
			DamageCauser->Tags.Remove("Attack|Common");
		}
	}	

	//데미지가 전체 체력의 20% 미만이면 넉백이 출력되지 않는다.
	if(DamageAmount >= GetCharacterStat().CharacterMaxHP * 0.2f)
	{
		TakeKnockBack(DamageCauser, DefaultKnockBackStrength);
	}

	GetWorldTimerManager().SetTimer(HitTimeOutTimerHandle, FTimerDelegate::CreateLambda([&]() {
		if (CharacterState.CharacterActionState == ECharacterActionType::E_Hit)
			ResetActionState();
	}), 1.0f, false, 0.5f);

	return damageAmount;
}

void AEnemyCharacterBase::TryAttack()
{
	Super::TryAttack();
}

void AEnemyCharacterBase::TrySkill()
{
	check(GetCurrentWeaponRef() != nullptr);

	if (CharacterState.CharacterActionState != ECharacterActionType::E_Skill
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;

	if (GetCurrentWeaponRef()->WeaponID == 0)
	{
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("스킬을 사용할 수 없습니다. ")), FColor::White);
		return;
	}

	//공격을 하고, 커서 위치로 Rotation을 조정
	Super::TrySkill();

	//Reset Weapon Skill Grade
	FWeaponStatStruct weaponStat = GetCurrentWeaponRef()->GetWeaponStat();
}

void AEnemyCharacterBase::Attack()
{
	Super::Attack();

	float attackSpeed = 0.5f;
	if(IsValid(GetCurrentWeaponRef()))
		attackSpeed = FMath::Min(1.5f, CharacterStat.CharacterAtkSpeed * GetCurrentWeaponRef()->GetWeaponStat().WeaponAtkSpeedRate);

	GetWorldTimerManager().SetTimer(AtkIntervalHandle, this, &ACharacterBase::ResetAtkIntervalTimer
		, 1.0f, false, FMath::Max(0.0f, 1.5f - attackSpeed));
}

void AEnemyCharacterBase::StopAttack()
{
	Super::StopAttack();
}

void AEnemyCharacterBase::Die()
{
	Super::Die();

	SpawnDeathItems();
	ClearAllTimerHandle();
	EnemyDeathDelegate.ExecuteIfBound(this);

	AAIControllerBase* aiControllerRef = Cast<AAIControllerBase>(Controller);

	if (IsValid(aiControllerRef))
	{
		aiControllerRef->ClearAllTimerHandle();
		aiControllerRef->UnPossess();
		aiControllerRef->Destroy();
	}
}

void AEnemyCharacterBase::SetDefaultWeapon()
{
	if (IsValid(GetInventoryRef()))
	{
		bool result = GetInventoryRef()->AddWeapon(DefaultWeaponID);
		if (result)
		{
			Cast<AAIControllerBase>(Controller)->UpdateNewWeapon();
		}
	}
}

void AEnemyCharacterBase::SetDefaultStat()
{
	CharacterStat.bInfinite = true;
}

void AEnemyCharacterBase::SpawnDeathItems()
{
	int32 totalSpawnItemCount = UKismetMathLibrary::RandomIntegerInRange(0, MaxSpawnItemCount);
	int32 trySpawnCount = 0; //스폰 시도를 한 횟수

	TArray<AItemBase*> spawnedItemList;

	UE_LOG(LogTemp, Warning, TEXT("totalSpawnItemCount %d"), totalSpawnItemCount);

	if (SpawnItemTypeList.IsValidIndex(0) && SpawnItemTypeList[0] == EItemCategoryInfo::E_Mission)
	{
		/*AItemBase* newItem = GamemodeRef->SpawnItemToWorld(SpawnItemIDList[0], GetActorLocation() + FMath::VRand() * 10.0f);
		if (IsValid(newItem))
		{
			spawnedItemList.Add(newItem);
			//newItem->Dele_MissionItemSpawned.BindUFunction(target, FName("TryAddMissionItem"));
		}*/
	}
	else
	{
		while (totalSpawnItemCount)
		{
			if (++trySpawnCount > totalSpawnItemCount * 3) break; //스폰할 아이템 개수의 3배만큼 시도

			const int32 itemIdx = UKismetMathLibrary::RandomIntegerInRange(0, SpawnItemIDList.Num() - 1);
			if (!SpawnItemTypeList.IsValidIndex(itemIdx) || !ItemSpawnProbabilityList.IsValidIndex(itemIdx)) continue;

			const uint8 itemType = (uint8)SpawnItemTypeList[itemIdx];
			const uint8 itemID = SpawnItemIDList[itemIdx];
			const float spawnProbability = ItemSpawnProbabilityList[itemIdx];

			if (FMath::RandRange(0.0f, 1.0f) <= spawnProbability)
			{
				AItemBase* newItem = GamemodeRef->SpawnItemToWorld(itemID, GetActorLocation() + FMath::VRand() * 10.0f);

				if (IsValid(newItem))
				{
					spawnedItemList.Add(newItem);
					totalSpawnItemCount -= 1;
				}
			}
		}
	}
	for (auto& targetItem : spawnedItemList)
	{
		targetItem->ActivateProjectileMovement();
		targetItem->ActivateItem();
	}
}

void AEnemyCharacterBase::SetFacialMaterialEffect(bool NewState)
{
	if (CurrentDynamicMaterial == nullptr) return;

	CurrentDynamicMaterial->SetScalarParameterValue(FName("EyeBrightness"), NewState ? 5.0f : 35.0f);
	CurrentDynamicMaterial->SetVectorParameterValue(FName("EyeColor"), NewState ? FColor::Red : FColor::Yellow);
	InitDynamicMeshMaterial(CurrentDynamicMaterial);
}

void AEnemyCharacterBase::Turn(float Angle)
{
	if (FMath::Abs(Angle) <= 0.05f)
	{
		CharacterState.TurnDirection = 0;
		return;
	}

	FRotator newRotation =  GetActorRotation();
	newRotation.Yaw += Angle;
	newRotation.Pitch = newRotation.Roll = 0.0f;
	SetActorRotation(newRotation);
	
	if (GetVelocity().Length() == 0.0f)
	{
		CharacterState.TurnDirection = (FMath::Sign(Angle) == 1 ? 2 : 1);
		GetWorldTimerManager().ClearTimer(TurnTimeOutTimerHandle);
		GetWorldTimerManager().SetTimer(TurnTimeOutTimerHandle, FTimerDelegate::CreateLambda([&]() {
			CharacterState.TurnDirection = 0;
		}), 1.0f, false, TURN_TIME_OUT_SEC);
		return;
	}
	CharacterState.TurnDirection = 0;
	return;
}

float AEnemyCharacterBase::PlayPreChaseAnimation()
{
	if (PreChaseAnimMontage == nullptr) return 0.0f;

	return PlayAnimMontage(PreChaseAnimMontage);
}

void AEnemyCharacterBase::ClearAllTimerHandle()
{
	Super::ClearAllTimerHandle();
	GetWorldTimerManager().ClearTimer(TurnTimeOutTimerHandle);
	GetWorldTimerManager().ClearTimer(HitTimeOutTimerHandle);
}