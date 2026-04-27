// Fill out your copyright notice in the Description page of Project Settings.
#include "EnemyCharacterBase.h"
#include "../Component/TargetingManagerComponent.h"
#include "../Component/WeaponComponentBase.h"
#include "../Component/SkillManagerComponentBase.h"
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
	FloatingHpBar->SetDrawSize({ 100.0f, 20.0f });
	FloatingHpBar->SetWidgetSpace(EWidgetSpace::Screen);
	FloatingHpBar->SetVisibility(false);

	EnemyStatInfoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ENEMY_STAT_INFO"));
	EnemyStatInfoWidget->SetupAttachment(GetCapsuleComponent());
	EnemyStatInfoWidget->SetWidgetSpace(EWidgetSpace::Screen);
	EnemyStatInfoWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	EnemyStatInfoWidget->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
	EnemyStatInfoWidget->SetWidgetSpace(EWidgetSpace::Screen);
	EnemyStatInfoWidget->SetVisibility(true);

	TargetingSupportWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TARGETING_SUPPORT"));
	TargetingSupportWidget->SetupAttachment(GetCapsuleComponent());
	TargetingSupportWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
	TargetingSupportWidget->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
	TargetingSupportWidget->SetRelativeScale3D(FVector(0.15f));
	TargetingSupportWidget->SetDrawSize({ 500.0f, 500.0f });
	TargetingSupportWidget->SetWidgetSpace(EWidgetSpace::Screen);
	TargetingSupportWidget->SetVisibility(false);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	this->Tags.Add("Enemy");
	this->Tags.Add("Easy");
}

void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultController();
	
	CharacterGameplayInfo.bUseDefaultStat = true;

	InitEnemyCharacter(CharacterID);
	SetDefaultWeapon();
	
	if (IsValid(SkillManagerComponent))
	{
		SkillManagerComponent->OnSkillDeactivated.Clear();
		SkillManagerComponent->OnSkillDeactivated.AddDynamic(this, &AEnemyCharacterBase::OnSkillDeactivated);
	}
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
	if (NewCharacterID == 0) return;
	if (EnemyStatTable == nullptr) return;

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
		newInfo.bUseDefaultStat = true;

		InitCharacterGameplayInfo(newInfo);
		CharacterGameplayInfo.bInfinite = true;
		SetDefaultWeapon();
	}

	InitFloatingHpWidget();
	InitEnemyStatInfoWidget();

	if (NewCharacterID == 1200)
	{
		UE_LOG(LogTemp, Warning, TEXT("Floating Widget 초기화"));
	}
}

void AEnemyCharacterBase::ResetAiBehaviorState()
{
	AAIControllerBase* aiControllerRef;
	aiControllerRef = Cast<AAIControllerBase>(Controller);

	if (IsValid(aiControllerRef) && aiControllerRef->GetBehaviorState() == EAIBehaviorType::E_Skill)
	{
		aiControllerRef->SetBehaviorState(EAIBehaviorType::E_Idle);
	}
}

void AEnemyCharacterBase::SetDefaultWeapon()
{
	WeaponComponent->InitWeapon(EnemyStat.DefaultWeaponID);
	if (IsValid(Controller) && EnemyStat.DefaultWeaponID != 0)
	{
		AAIControllerBase* controllerRef = Cast<AAIControllerBase>(Controller);
		if (IsValid(controllerRef))
		{
			controllerRef->UpdateNewWeapon();
		}
	}
}

void AEnemyCharacterBase::TakeKnockBack(float KnockbackForce, float KnockbackResist)
{
	if (IsActorBeingDestroyed()) return;
	if (GetIsActionActive(ECharacterActionType::E_Die)) return;

	AAIControllerBase* aiControllerRef = nullptr;
	aiControllerRef = Cast<AAIControllerBase>(Controller);

	float effectiveKnockback = KnockbackForce * (1 - KnockbackResist);
	const float knockbackThreshold = 750;
	
	if (IsValid(aiControllerRef) && CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Skill
		&& aiControllerRef->GetBehaviorState() != EAIBehaviorType::E_Skill)
	{
		if (GetIsActionActive(ECharacterActionType::E_Stun))
		{
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		}
		if (effectiveKnockback >= knockbackThreshold)
		{
			if (GetWorldTimerManager().IsTimerActive(DamageAIDelayTimer))
			{
				GetWorldTimerManager().ClearTimer(DamageAIDelayTimer);
			}
			aiControllerRef->DeactivateAI();
			SetActionState(ECharacterActionType::E_KnockedDown);
			PlayKnockBackAnimMontage();
			GetWorldTimerManager().SetTimer(DamageAIDelayTimer, aiControllerRef, &AAIControllerBase::ActivateAI, 1.0f, false, 5.0f);
		}
		else if (effectiveKnockback < knockbackThreshold)
		{
			if (GetIsActionActive(ECharacterActionType::E_KnockedDown)) SetActionState(ECharacterActionType::E_Idle);
			PlayHitAnimMontage();
		}
	}

	//interp location backward (enemycharacter location - playercharacter location)
	FVector playerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	playerLocation.Z = GetActorLocation().Z; //Keep Z axis to prevent falling down
	
	float knockbackDistance = UKismetMathLibrary::MapRangeClamped(effectiveKnockback, 0.0f, knockbackThreshold, 0.0f, 200.0f);

	//pawn, worldstatic, worlddynamic
	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypeList = {
		EObjectTypeQuery::ObjectTypeQuery1, //WorldStatic
		EObjectTypeQuery::ObjectTypeQuery2, //WorldDynamic
		EObjectTypeQuery::ObjectTypeQuery3  //Pawn
	};

	if (knockbackDistance <= 0.0f) return;
	SetLocationWithInterp(GetActorLocation(), GetActorLocation() - (playerLocation - GetActorLocation()).GetSafeNormal() * knockbackDistance, objectTypeList, 1.0f, false, true, true);
}

bool AEnemyCharacterBase::ApplyDebuffStack(FDebuffInfoStruct DebuffInfo, AActor* Causer)
{
	bool result = Super::ApplyDebuffStack(DebuffInfo, Causer);
	if (result && DebuffInfo.Type == ECharacterDebuffType::E_Stun && Causer->ActorHasTag("Player"))
	{
		AAIControllerBase* aiControllerRef = nullptr;
		aiControllerRef = Cast<AAIControllerBase>(Controller);
		if (aiControllerRef->GetBehaviorState() == EAIBehaviorType::E_Skill) aiControllerRef->SetBehaviorState(EAIBehaviorType::E_Idle);
	}
	return result;
}

float AEnemyCharacterBase::TakeDebuffDamage(ECharacterDebuffType DebuffType, float DamageAmount, AActor* Causer)
{
	float dmamageAmount = Super::TakeDebuffDamage(DebuffType, DamageAmount, Causer);
	
	FloatingHpBar->SetVisibility(CharacterGameplayInfo.CurrentHP > 0.0f);

	return dmamageAmount;
}

float AEnemyCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	APawn* causerPawn = IsValid(EventInstigator) ? EventInstigator->GetPawn() : nullptr;

	if (!IsValid(DamageCauser) || !IsValid(causerPawn) || !causerPawn->ActorHasTag("Player") || damageAmount <= 0.0f || CharacterGameplayInfo.bIsInvincibility)
	{
		return 0.0f;
	}

	EnemyDamageDelegate.ExecuteIfBound(causerPawn);
	
	SetInstantHpWidgetVisibility();
	FloatingHpBar->SetVisibility(CharacterGameplayInfo.CurrentHP > 0.0f);


	// 250928 적 캐릭터가 스킬 상태일 때 UpdateDamgeAmount를 호출하지 않는 빨간약, 왜 스킬 상태일 때 바로 리턴했는지 모르겠음
	// (아마 스킬상태 일 때 Hit시 스킬 끊기지 않게 하기 위해 추가했던것으로 추정)
	//if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Skill) return damageAmount;

	if (DamageCauser->ActorHasTag("Projectile") && IsValid(Cast<AMainCharacterBase>(causerPawn)))
	{
		Cast<AMainCharacterBase>(causerPawn)->UpdateDamageAmount(-1.0f, damageAmount);
	}
	else if (DamageCauser->ActorHasTag("Player"))
	{
		//apply fov hit effect by damage amountd
		float fovValue = 90.0f + FMath::Clamp(DamageAmount/100.0f, 0.0f, 1.0f) * 45.0f;
		if (Cast<AMainCharacterBase>(DamageCauser)->GetCharacterGameplayInfo().bIsDownwardAttacking)
		{
			fovValue = 120.0f;
		}
		Cast<AMainCharacterBase>(DamageCauser)->SetFieldOfViewWithInterp(fovValue, 5.0f, true);
		Cast<AMainCharacterBase>(DamageCauser)->UpdateDamageAmount(damageAmount);
	}
	
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Hit)
	{
		GetWorldTimerManager().SetTimer(HitTimeOutTimerHandle, this, &AEnemyCharacterBase::ResetActionStateForTimer, 1.0f, false, CharacterID == 1200 ? 0.1f : 0.5f);
	}

	//Stop AI Logic And Set Reactivation event
	//Set Rotation To Causer
	//250825 Exception Handling for Boss (Stop AI Logic And Set Reactivation event, Rotating towards the actor that the damage causer)
	AAIControllerBase* aiControllerRef = Cast<AAIControllerBase>(Controller);
	if (!ActorHasTag("Boss") && ActorHasTag("Character"))
	{
		if (IsValid(aiControllerRef) && aiControllerRef->GetBehaviorState() != EAIBehaviorType::E_Skill
			&& CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_KnockedDown)
		{
			aiControllerRef->DeactivateAI();
			GetWorldTimerManager().ClearTimer(DamageAIDelayTimer);
			GetWorldTimerManager().SetTimer(DamageAIDelayTimer, aiControllerRef, &AAIControllerBase::ActivateAI, 1.0f, false, 1.5f);
		}
		if (IsValid(causerPawn))
		{
			FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), causerPawn->GetActorLocation());
			newRotation.Pitch = newRotation.Roll = 0.0f;
			SetActorRotation(newRotation);
		}
	}
	
	//Check IgnoreHit
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_KnockedDown)
	{
		bool ignoreHitResult = CalculateIgnoreHitProbability(CharacterGameplayInfo.HitCounter);
		if (ignoreHitResult)
		{
			aiControllerRef->SetBehaviorState(EAIBehaviorType::E_Skill);
		}
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
	EnemyStatInfoWidget->SetVisibility(false);

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
	/* Boss 레벨에서의 아이템 드랍 처리 ===============
	* 보스레벨에서의 보스를 제외한 적들은 아이템 드랍을 하지 않음
	* GetWorld()->GetMapName()함수 사용 시 Persistent Level인 UEDPIE_0_Chapter01_Main으로 나오는 문제 때문에
	* ChapterManager를 통해 현재 스테이지 정보를 얻어옴
	*/
	ABackStreetGameModeBase* gamemodeRef = (ABackStreetGameModeBase*)GetWorld()->GetAuthGameMode();
	if (IsValid(gamemodeRef) && IsValid(gamemodeRef->GetChapterManagerRef()))
	{
		EStageCategoryInfo currentStageType = GamemodeRef->GetChapterManagerRef()->StageManagerComponent->GetCurrentStageInfo().StageType;
		if (currentStageType == EStageCategoryInfo::E_Boss) return;
	}
	
	FEnemyDropInfoStruct dropInfo = EnemyStat.DropInfo;

	int32 totalSpawnItemCount = 1;

	// 럭키 드롭 어빌리티 ==================
	TWeakObjectPtr<ACharacterBase> playerRef = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (playerRef.IsValid())
	{
		const float probability = playerRef.Get()->GetStatProbabilityValue(ECharacterStatType::E_LuckyDrop);
		const float statValue = playerRef.Get()->GetStatTotalValue(ECharacterStatType::E_LuckyDrop);

		if (statValue > 1.0f && FMath::FRand() <= probability)
		{
			totalSpawnItemCount *= (int32)statValue;
		}	
	}	

	// 실제 스폰 처리 =====================
	int32 trySpawnCount = 0; //스폰 시도를 한 횟수
	TArray<AItemBase*> spawnedItemList;

	while (totalSpawnItemCount)
	{
		if (++trySpawnCount > totalSpawnItemCount * 3) break; //스폰할 아이템 개수의 3배만큼 시도

		const int32 itemIdx = UKismetMathLibrary::RandomIntegerInRange(0, dropInfo.SpawnItemIDList.Num() - 1);
		if (!dropInfo.SpawnItemTypeList.IsValidIndex(itemIdx)
			|| !dropInfo.ItemSpawnProbabilityList.IsValidIndex(itemIdx)) continue;

		const uint8 itemType = (uint8)dropInfo.SpawnItemTypeList[itemIdx];
		const int32 itemID = 1; //조합 재료만 나오도록 강제 250210 //dropInfo.SpawnItemIDList[itemIdx];
		const float spawnProbability = dropInfo.ItemSpawnProbabilityList[itemIdx];

		if (FMath::RandRange(0.0f, 1.0f) <= spawnProbability)
		{
			AItemBase* newItem = GamemodeRef->SpawnItemToWorld(itemID, GetActorLocation() + FVector(0.0f, 0.0f, 200.0f));

			if (IsValid(newItem))
			{
				spawnedItemList.Add(newItem);
				totalSpawnItemCount -= 1;
			}
		}
	}

	for (auto& targetItem : spawnedItemList)
	{
		targetItem->SetItemAmount(1);
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

	if (FMath::FRand() <= result)
	{
		return true;
	}
	else return false;
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