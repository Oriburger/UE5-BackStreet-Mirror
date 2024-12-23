﻿// Fill out your copyright notice in the Description page of Project Settings.
#include "EnemyCharacterBase.h"
#include "../Component/TargetingManagerComponent.h"
#include "../Component/WeaponComponentBase.h"
#include "../Component/SkillManagerComponentBase.h"
#include "../Component/EnemySkillManagerComponent.h"
#include "../MainCharacter/MainCharacterBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/MapSystem/NewChapterManagerBase.h"
#include "../../System/MapSystem/StageManagerComponent.h"
#include "../../System/AssetSystem/AssetManagerBase.h"
#include "../../System/AISystem/AIControllerBase.h"
#include "../../Item/ItemBase.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
	FloatingHpBar->SetVisibility(false);

	TargetingSupportWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TARGETING_SUPPORT"));
	TargetingSupportWidget->SetupAttachment(GetCapsuleComponent());
	TargetingSupportWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
	TargetingSupportWidget->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
	TargetingSupportWidget->SetRelativeScale3D(FVector(0.15f));
	TargetingSupportWidget->SetDrawSize({ 500.0f, 500.0f });
	TargetingSupportWidget->SetVisibility(false);

	SkillManagerComponent = CreateDefaultSubobject<UEnemySkillManagerComponent>(TEXT("SKILL_MANAGER_"));
	SkillManagerComponentRef = SkillManagerComponent;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	this->Tags.Add("Enemy");
	this->Tags.Add("Easy");
}

void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultController();
	InitEnemyCharacter(CharacterID);
	SetDefaultWeapon();
}

void AEnemyCharacterBase::InitAsset(int32 NewCharacterID)
{
	Super::InitAsset(NewCharacterID);

	if (IsValid(AssetSoftPtrInfo.AIControllerClass))
	{
		AIControllerClass = AssetSoftPtrInfo.AIControllerClass;
	}
}

void AEnemyCharacterBase::InitEnemyCharacter(int32 NewCharacterID)
{
	//Read from dataTable
	FString rowName = FString::FromInt(NewCharacterID);
	FEnemyStatStruct* newStat = EnemyStatTable->FindRow<FEnemyStatStruct>(FName(rowName), rowName);
	AssetSoftPtrInfo.CharacterID = CharacterID = NewCharacterID;
	
	if (newStat != nullptr)
	{
		EnemyStat = *newStat;

		//Set CharacterStat with setting default additional stat bInfinite (infinite use of ammo)
		FCharacterGameplayInfo newInfo = FCharacterGameplayInfo(EnemyStat.DefaultStat);
		newInfo.CharacterID = NewCharacterID;
		InitCharacterGameplayInfo(newInfo);
		CharacterGameplayInfo.bInfinite = true;
		SetDefaultWeapon();
	}

	if (NewCharacterID != 0)
	{
		SkillManagerComponent->ClearAllSkill();
		for (int32& skillID : EnemyStat.EnemySkillIDList)
		{
			if (skillID != 0)
			{
				bool result = SkillManagerComponent->AddSkill(skillID);
				UE_LOG(LogTemp, Warning, TEXT("Add Skill %d --- %d"), skillID, (int32)result);
			}
		}
	}

	InitFloatingHpWidget();
	InitTargetingSupportingWidget();

	if (NewCharacterID == 1200)
	{
		UE_LOG(LogTemp, Warning, TEXT("Floating Widget 초기화"));
	}
}

void AEnemyCharacterBase::SetDefaultWeapon()
{
	WeaponComponent->InitWeapon(EnemyStat.DefaultWeaponID);
	if (IsValid(Controller))
	{
		Cast<AAIControllerBase>(Controller)->UpdateNewWeapon();
	}
}

float AEnemyCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!IsValid(DamageCauser) || !DamageCauser->ActorHasTag("Player") || damageAmount <= 0.0f || CharacterGameplayInfo.bIsInvincibility) return 0.0f;

	EnemyDamageDelegate.ExecuteIfBound(DamageCauser);
	SetInstantHpWidgetVisibility();
	
	FloatingHpBar->SetVisibility(CharacterGameplayInfo.CurrentHP > 0.0f);

	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Skill) return damageAmount;

	if (DamageCauser->ActorHasTag("Player"))
	{
		//apply fov hit effect by damage amount
		float fovValue = 90.0f + FMath::Clamp(DamageAmount/100.0f, 0.0f, 1.0f) * 45.0f;
		if (Cast<AMainCharacterBase>(DamageCauser)->GetCharacterGameplayInfo().bIsDownwardAttacking)
		{
			fovValue = 120.0f;
		}
		Cast<AMainCharacterBase>(DamageCauser)->SetFieldOfViewWithInterp(fovValue, 5.0f, true);
	}	
	
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Hit)
	{
		GetWorldTimerManager().SetTimer(HitTimeOutTimerHandle, this, &AEnemyCharacterBase::ResetActionStateForTimer, 1.0f, false, CharacterID == 1200 ? 0.1f : 0.5f);
	}

	//Stop AI Logic And Set Reactivation event
	AAIControllerBase* aiControllerRef = Cast<AAIControllerBase>(Controller);
	if (IsValid(aiControllerRef) && CharacterID != 1200 && aiControllerRef->GetBehaviorState() != EAIBehaviorType::E_Skill)
	{
		aiControllerRef->DeactivateAI();
		GetWorldTimerManager().ClearTimer(DamageAIDelayTimer);
		GetWorldTimerManager().SetTimer(DamageAIDelayTimer, aiControllerRef, &AAIControllerBase::ActivateAI, 1.0f, false, 1.5f);
	}

	//Set Rotation To Causer
	if (!DamageCauser->ActorHasTag("Boss"))
	{
		FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), DamageCauser->GetActorLocation());
		newRotation.Pitch = newRotation.Roll = 0.0f;
		SetActorRotation(newRotation);
	}
	
	//Check IgnoreHit
	bool ignoreHitResult = CalculateIgnoreHitProbability(CharacterGameplayInfo.HitCounter);
	if (ignoreHitResult)
	{	
		aiControllerRef->SetBehaviorState(EAIBehaviorType::E_Skill);
	}

	return damageAmount;
}

void AEnemyCharacterBase::TryAttack()
{
	Super::TryAttack();
}

bool AEnemyCharacterBase::TrySkill(int32 SkillID)
{
	return Super::TrySkill(SkillID);
}

void AEnemyCharacterBase::Attack()
{
	Super::Attack();

	const float attackSpeed = FMath::Min(1.5f, CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_NormalAttackSpeed) * WeaponComponent->WeaponStat.WeaponAtkSpeedRate);

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

	FloatingHpBar->SetVisibility(false);
	TargetingSupportWidget->SetVisibility(false);

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
			//임시로 오프셋 넣어둠. 언젠가 Linetrace로 바닥 맞춰주는 로직 추가할 것.
			AItemBase* newItem = GamemodeRef->SpawnItemToWorld(itemID, GetActorLocation() + FMath::VRand() * 10.0f - FVector(0.0f, 0.0f, 50.0f));
			
			if (IsValid(newItem))
			{
				spawnedItemList.Add(newItem);
				totalSpawnItemCount -= 1;
			}
		}
	}

	ACharacterBase* playerRef = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	for (auto& targetItem : spawnedItemList)
	{
		uint8 newAmount = 1;
		if (IsValid(playerRef) && playerRef->GetStatProbabilityValue(ECharacterStatType::E_LuckyDrop) >= FMath::FRandRange(0.0f, 1.0f))
		{
			newAmount = 2;
		}
		targetItem->SetItemAmount(newAmount);
		GamemodeRef.Get()->RegisterActorToStageManager(targetItem);
		targetItem->ActivateItem();
		//targetItem->ActivateProjectileMovement(); //텍스쳐만 있는 아이템의 경우에는 바닥을 뚫는 문제가 있음, 추후 개선 필요
	}
}

void AEnemyCharacterBase::ResetActionStateForTimer()
{
	ResetActionState(true);
}

void AEnemyCharacterBase::SetFacialMaterialEffect(bool NewState)
{
	//if (CurrentDynamicMaterialList.IsEmpty()) return;
	//HardCoding
	//CurrentDynamicMaterialList[0]->SetScalarParameterValue(FName("EyeBrightness"), NewState ? 5.0f : 35.0f);
	//CurrentDynamicMaterialList[0]->SetVectorParameterValue(FName("EyeColor"), NewState ? FColor::Red : FColor::Yellow);
	//InitDynamicMaterialList(DynamicMaterialList);
}

void AEnemyCharacterBase::InitTargetingSupportingWidget_Implementation()
{
	AMainCharacterBase* playerCharacter = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (IsValid(playerCharacter) && IsValid(playerCharacter->TargetingManagerComponent))
	{
		playerCharacter->TargetingManagerComponent->OnTargetUpdated.AddDynamic(this, &AEnemyCharacterBase::OnTargetUpdated);
		playerCharacter->TargetingManagerComponent->OnTargetingActivated.AddDynamic(this, &AEnemyCharacterBase::OnTargetingActivated);
	}
}

float AEnemyCharacterBase::PlayPreChaseAnimation()
{
	if (AssetHardPtrInfo.PointMontageList.Num() <= 0 || !IsValid(AssetHardPtrInfo.PointMontageList[0])) return 0.0f;
	return PlayAnimMontage(AssetHardPtrInfo.PointMontageList[0]);
}

void AEnemyCharacterBase::KnockDown()
{
	Super::KnockDown();

	AAIControllerBase* aiControllerRef = Cast<AAIControllerBase>(Controller);

	if (IsValid(aiControllerRef))
	{
		aiControllerRef->DeactivateAI();
		GetWorldTimerManager().ClearTimer(DamageAIDelayTimer);
		GetWorldTimerManager().SetTimer(DamageAIDelayTimer, aiControllerRef, &AAIControllerBase::ActivateAI, 1.0f, false, 5.0f);
	}
}

void AEnemyCharacterBase::StandUp()
{
	Super::StandUp();
}

bool AEnemyCharacterBase::CalculateIgnoreHitProbability(int32 HitCounter)
{
	float baseProb = 0.05f, maxProb = 0.9f;
	float increment = EnemyStat.DefaultStat.DefaultDefense * 0.2f;
	float ignoreHitProbability = baseProb + (HitCounter * increment);

	float result = UKismetMathLibrary::FMin(maxProb, ignoreHitProbability);
	UE_LOG(LogTemp, Warning, TEXT("CalcIgnoreHit result : %f"), result);

	if (FMath::FRand() <= result)
	{
		return true;
	}
	else return false;
	return false;
}

void AEnemyCharacterBase::SetInstantHpWidgetVisibility()
{
	FloatingHpBar->SetVisibility(true);

	//Clear and invalidate timer
	GetWorldTimerManager().ClearTimer(HpWidgetAutoDisappearTimer);
	HpWidgetAutoDisappearTimer.Invalidate();

	//Set new timer
	FTimerDelegate disappearEvent;
	disappearEvent.BindUFunction(FloatingHpBar, FName("SetVisibility"), false);
	GetWorldTimerManager().SetTimer(HpWidgetAutoDisappearTimer, disappearEvent, 10.0f, false);
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