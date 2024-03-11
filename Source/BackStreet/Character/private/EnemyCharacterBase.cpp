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
	this->Tags.Add("Easy");

	
	static ConstructorHelpers::FObjectFinder<UDataTable> statTableFinder(TEXT("/Game/Character/EnemyCharacter/Data/D_EnemyStatDataTable.D_EnemyStatDataTable"));
	
	//if stat table is not identified on editor, crash event is force activated.
	checkf(statTableFinder.Succeeded(), TEXT("Enemy Stat Table 탐색에 실패했습니다."));
	if(statTableFinder.Succeeded())
	{
		EnemyStatTable = statTableFinder.Object;
	}
}

void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultController();

	InitEnemyCharacter(CharacterID);

	SetDefaultWeapon();
	InitDynamicMeshMaterial(GetMesh()->GetMaterial(0));
}

void AEnemyCharacterBase::InitEnemyCharacter(int32 NewCharacterID)
{
	// Read from dataTable
	FString rowName = FString::FromInt(NewCharacterID);
	FEnemyStatStruct* newStat = EnemyStatTable->FindRow<FEnemyStatStruct>(FName(rowName), rowName);
	AssetInfo.CharacterID = CharacterID = NewCharacterID;
	

	if (newStat != nullptr)
	{
		EnemyStat = *newStat;

		//Set CharacterStat with setting default additional stat bInfinite (infinite use of ammo)
		UpdateCharacterStat(EnemyStat.CharacterStat);
		CharacterStat.bInfinite = true;
		CharacterState.CharacterCurrHP = EnemyStat.CharacterStat.DefaultHP;
		SetDefaultWeapon();
	}

	InitFloatingHpWidget();
	if (NewCharacterID == 1200)
	{
		UE_LOG(LogTemp, Warning, TEXT("Floating Widget 초기화"));
	}
}

void AEnemyCharacterBase::SetDefaultWeapon()
{
	if (IsValid(GetInventoryRef()))
	{
		bool result = GetInventoryRef()->AddWeapon(EnemyStat.DefaultWeaponID);
		if (result)
		{
			Cast<AAIControllerBase>(Controller)->UpdateNewWeapon();
		}
	}
}

float AEnemyCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!IsValid(DamageCauser) || !DamageCauser->ActorHasTag("Player") || damageAmount <= 0.0f) return 0.0f;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitImpactSound, GetActorLocation());
	EnemyDamageDelegate.ExecuteIfBound(DamageCauser);

	if (DamageCauser->ActorHasTag("Player"))
	{
		if (DamageCauser->ActorHasTag("Attack|Common"))
		{
			Cast<AMainCharacterBase>(DamageCauser)->AddSkillGauge();
			DamageCauser->Tags.Remove("Attack|Common");
		}
	}	

	if (CharacterState.CharacterActionState == ECharacterActionType::E_Hit)
	{
		GetWorldTimerManager().SetTimer(HitTimeOutTimerHandle, this, &AEnemyCharacterBase::ResetActionStateForTimer, 1.0f, false, CharacterID == 1200 ? 0.1f : 0.5f);
	}

	//Stop AI Logic And Set Reactivation event
	AAIControllerBase* aiControllerRef = Cast<AAIControllerBase>(Controller);
	if (IsValid(aiControllerRef) && CharacterID != 1200)
	{
		aiControllerRef->DeactivateAI();
		GetWorldTimerManager().ClearTimer(DamageAIDelayTimer);
		GetWorldTimerManager().SetTimer(DamageAIDelayTimer, aiControllerRef, &AAIControllerBase::ActivateAI, 1.0f, false, 1.5f);
	}

	//Set Rotation To Causer
	FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), DamageCauser->GetActorLocation());
	newRotation.Pitch = newRotation.Roll = 0.0f;
	SetActorRotation(newRotation);

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
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("The Skill Is Not Available")), FColor::White);
		return;
	}

	Super::TrySkill();
}

void AEnemyCharacterBase::Attack()
{
	Super::Attack();

	float attackSpeed = 0.5f;
	if(IsValid(GetCurrentWeaponRef()))
		attackSpeed = FMath::Min(1.5f, CharacterStat.DefaultAttackSpeed * GetCurrentWeaponRef()->GetWeaponStat().WeaponAtkSpeedRate);

	GetWorldTimerManager().SetTimer(AtkIntervalHandle, this, &ACharacterBase::ResetAtkIntervalTimer
		, 1.0f, false, FMath::Max(0.0f, 1.5f - attackSpeed));

	//Temporary code for indieGo 
	if (CharacterID == 1200)
	{
		BossNearAttack();
	}
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

void AEnemyCharacterBase::SpawnDeathItems()
{
	FEnemyDropInfoStruct dropInfo = EnemyStat.DropInfo;

	int32 totalSpawnItemCount = UKismetMathLibrary::RandomIntegerInRange(0, dropInfo.MaxSpawnItemCount);
	int32 trySpawnCount = 0; //스폰 시도를 한 횟수

	TArray<AItemBase*> spawnedItemList;

	if (dropInfo.SpawnItemTypeList.IsValidIndex(0) 
		&& dropInfo.SpawnItemTypeList[0] == EItemCategoryInfo::E_Mission)
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

			const int32 itemIdx = UKismetMathLibrary::RandomIntegerInRange(0, dropInfo.SpawnItemIDList.Num() - 1);
			if (!dropInfo.SpawnItemTypeList.IsValidIndex(itemIdx)
				|| !dropInfo.ItemSpawnProbabilityList.IsValidIndex(itemIdx)) continue;

			const uint8 itemType = (uint8)dropInfo.SpawnItemTypeList[itemIdx];
			const int32 itemID = dropInfo.SpawnItemIDList[itemIdx];
			const float spawnProbability = dropInfo.ItemSpawnProbabilityList[itemIdx];

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

void AEnemyCharacterBase::ResetActionStateForTimer()
{
	ResetActionState(true);
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
		GetWorldTimerManager().SetTimer(TurnTimeOutTimerHandle, this, &AEnemyCharacterBase::ResetTurnAngle, 1.0f, false, TURN_TIME_OUT_SEC);
		return;
	}
	ResetTurnAngle();
	return;
}

float AEnemyCharacterBase::PlayPreChaseAnimation()
{
	if (PreChaseAnimMontage == nullptr) return 0.0f;
	return PlayAnimMontage(PreChaseAnimMontage);
}

void AEnemyCharacterBase::ResetTurnAngle()
{
	CharacterState.TurnDirection = 0;
}

void AEnemyCharacterBase::ClearAllTimerHandle()
{
	Super::ClearAllTimerHandle();
	GetWorldTimerManager().ClearTimer(TurnTimeOutTimerHandle);
	GetWorldTimerManager().ClearTimer(HitTimeOutTimerHandle);
	GetWorldTimerManager().ClearTimer(DamageAIDelayTimer);
	TurnTimeOutTimerHandle.Invalidate();
	HitTimeOutTimerHandle.Invalidate();
	DamageAIDelayTimer.Invalidate();
}