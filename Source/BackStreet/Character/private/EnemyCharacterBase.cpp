// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/EnemyCharacterBase.h"
#include "../../AISystem/public/AIControllerBase.h"
#include "../public/CharacterInfoStruct.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/ItemBase.h"
#include "../../StageSystem/public/StageInfoStruct.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
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

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	this->Tags.Add("Enemy");

	static ConstructorHelpers::FObjectFinder<UDataTable> statTableFinder(TEXT("/Game/Character/EnemyCharacter/Data/D_EnemyStatDataTable.D_EnemyStatDataTable"));
	//클래스를 제대로 명시하지 않았으면 크래시를 띄움
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

	InitEnemyStat(CharacterID);
	InitFloatingHpWidget();
	
	SetDefaultWeapon();
	InitDynamicMeshMaterial(GetMesh()->GetMaterial(0));
}

void AEnemyCharacterBase::InitEnemyStat(int32 NewCharacterID)
{
	//DataTable로 부터 Read
	FString rowName = FString::FromInt(NewCharacterID);
	FEnemyStatStruct* newStat = EnemyStatTable->FindRow<FEnemyStatStruct>(FName(rowName), rowName);

	UE_LOG(LogTemp, Warning, TEXT("Init Enemy Stat %d"), NewCharacterID);
	if (newStat != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("---Success"));
		//기본 스탯 설정
		UpdateCharacterStat(newStat->EnemyStat);
		CharacterState.CharacterCurrHP = CharacterStat.CharacterMaxHP;
		GetCharacterMovement()->MaxWalkSpeed = CharacterStat.CharacterMoveSpeed;
		CharacterStat.bInfinite = true;

		//드롭 아이템 설정
		ItemSpawnProbabilityList = newStat->ItemSpawnProbabilityList;
		MaxSpawnItemCount = newStat->MaxSpawnItemCount;
		SpawnItemIDList = newStat->SpawnItemIDList;
		SpawnItemTypeList = newStat->SpawnItemTypeList;
		
		//기본 무기 설정
		DefaultWeaponID = newStat->DefaultWeaponID;
		SetDefaultWeapon();
	}
}

float AEnemyCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!IsValid(DamageCauser) || !DamageCauser->ActorHasTag("Player") || damageAmount <= 0.0f) return 0.0f;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitImpactSound, GetActorLocation());
	EnemyDamageDelegate.ExecuteIfBound(DamageCauser);

	//const float knockBackStrength = 100000.0f;
	

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