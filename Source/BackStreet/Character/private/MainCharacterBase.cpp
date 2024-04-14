// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/MainCharacterBase.h"
#include "../public/MainCharacterController.h"
#include "../../Global/public/BackStreetGameInstance.h"
#include "../public/AbilityManagerBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Item/public/ThrowWeaponBase.h"
#include "../../Item/public/WeaponInventoryBase.h"
#include "../../Item/public/ItemBase.h"
#include "../../Item/public/ItemBoxBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../StageSystem/public/ChapterManagerBase.h"
#include "../../StageSystem/public/StageData.h"
#include "../../StageSystem/public/GateBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AudioComponent.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../../Item/public/RewardBoxBase.h"
#include "../../CraftingSystem/public/CraftBoxBase.h"
#include "Animation/AnimMontage.h"
#include "../../Global/public/AssetManagerBase.h"
#include "../../Global/public/SkillManagerBase.h"
#define MAX_CAMERA_BOOM_LENGTH 1450.0f
#define MIN_CAMERA_BOOM_LENGTH 250.0f
#define MAX_THROW_DISTANCE 1200.0f //AThrowWeaponBase와 통일 (추후 하나의 파일로 통합 예정)

// Sets default values
AMainCharacterBase::AMainCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//SetActorTickInterval(0.1f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CAMERA_BOOM"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 1400.0f;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->SetWorldRotation({ -45.0f, 0.0f, 0.0f });

	FollowingCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FOLLOWING_CAMERA"));
	FollowingCamera->SetupAttachment(CameraBoom);
	FollowingCamera->bAutoActivate = false;

	BuffNiagaraEmitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BUFF_EFFECT"));
	BuffNiagaraEmitter->SetupAttachment(GetMesh());
	BuffNiagaraEmitter->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f));
	BuffNiagaraEmitter->bAutoActivate = false;

	DirectionNiagaraEmitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DIRECTION_EFFECT"));
	DirectionNiagaraEmitter->SetupAttachment(GetMesh());
	DirectionNiagaraEmitter->SetRelativeRotation({ 0.0f, 90.0f, 0.0f });

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SOUND"));

	GetCapsuleComponent()->OnComponentHit.AddUniqueDynamic(this, &AMainCharacterBase::OnCapsuleHit);

	this->bUseControllerRotationYaw = false;
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->RotationRate = { 0.0f, 0.0f, 750.0f };
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GravityScale = 2.5f;

	this->Tags.Add("Player");
}

// Called when the game starts or when spawned
void AMainCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	PlayerControllerRef = Cast<AMainCharacterController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	AbilityManagerRef = NewObject<UAbilityManagerBase>(this, UAbilityManagerBase::StaticClass(), FName("AbilityfManager"));
	AbilityManagerRef->InitAbilityManager(this);

	UBackStreetGameInstance* GameInstance = Cast<UBackStreetGameInstance>(GetGameInstance());

	//Load SaveData
	if (GameInstance->LoadGameSaveData(SavedData)) return;
	else
	{
		GameInstance->SaveGameData(FSaveData());
	}
	SetCharacterStatFromSaveData();
	InitCharacterState();

}

// Called every frame
void AMainCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UpdateWallThroughEffect();
}

void AMainCharacterBase::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsValid(OtherActor) || OtherActor->IsActorBeingDestroyed() || !OtherActor->ActorHasTag("Character")) return;

	FVector impulse = OtherActor->GetActorLocation() - GetActorLocation();
	impulse.Z = 0;
	impulse.Normalize();
	impulse = impulse * ((GetVelocity().Length() == 0.0f ? 50.0f : 500.0f) * 100.0f);
	Cast<ACharacterBase>(OtherActor)->GetCharacterMovement()->AddImpulse(impulse);
}


// Called to bind functionality to input
void AMainCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Move);

		//Rolling
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Roll);

		//Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryAttack);

		//Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryReload);

		//Zoom
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::ZoomIn);

		//Throw
		EnhancedInputComponent->BindAction(ThrowReadyAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::ReadyToThrow);
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Throw);

		//Interaction
		EnhancedInputComponent->BindAction(InvestigateAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryInvestigate);

		//Inventory
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::SwitchToNextWeapon);
		EnhancedInputComponent->BindAction(DropWeaponAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::DropWeapon);

		//SubWeapon
		EnhancedInputComponent->BindAction(PickSubWeaponAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::PickSubWeapon);
	}
}

void AMainCharacterBase::ReadyToThrow()
{
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (!IsValid(GetCurrentWeaponRef()) || GetCurrentWeaponRef()->GetWeaponType() != EWeaponType::E_Throw) return;
	if (!Cast<AThrowWeaponBase>(GetCurrentWeaponRef())->GetCanThrow()) return; //딜레이 중이라면 반환
	if (GetCurrentWeaponRef()->WeaponID == 0) return;

	CharacterState.CharacterActionState = ECharacterActionType::E_Throw;
	SetAimingMode(true);
	GetWorldTimerManager().SetTimer(AimingTimerHandle, this, &AMainCharacterBase::UpdateAimingState, 0.01f, true);
}

void AMainCharacterBase::Throw()
{
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Throw) return;

	ResetActionState();
	SetAimingMode(false);

	GetWorldTimerManager().ClearTimer(AimingTimerHandle);
	if (IsValid(GetCurrentWeaponRef()) && GetCurrentWeaponRef()->GetWeaponType() == EWeaponType::E_Throw)
	{
		Cast<AThrowWeaponBase>(GetCurrentWeaponRef())->Throw();
	}
}

void AMainCharacterBase::SetAimingMode(bool bNewState)
{
	bIsAiming = bNewState;	
	GetCharacterMovement()->bOrientRotationToMovement = !bNewState;
	this->bUseControllerRotationYaw = bNewState;
}

void AMainCharacterBase::UpdateAimingState()
{	
	if (!IsValid(GetCurrentWeaponRef())) return;
	if (GetCurrentWeaponRef()->GetWeaponType() != EWeaponType::E_Throw) return;

	RotateToCursor();
	Cast<AThrowWeaponBase>(GetCurrentWeaponRef())->CalculateThrowDirection(GetThrowDestination());

	FPredictProjectilePathResult predictProjectilePathResult = Cast<AThrowWeaponBase>(GetCurrentWeaponRef())->GetProjectilePathPredictResult();
	
	//Draw projectile path with spline
	TArray<FVector> pathPointList;
	FVector cursorLocation = PlayerControllerRef.Get()->GetCursorDeprojectionWorldLocation();
	for (auto& pathData : predictProjectilePathResult.PathData)
	{
		if (!pathPointList.IsEmpty())
			pathPointList.Add((pathPointList[pathPointList.Num() - 1] + pathData.Location) / 2);
		pathPointList.Add(pathData.Location);
		//UE_LOG(LogTemp, Warning, TEXT("%s, %s"), *cursorLocation.ToString(), *pathData.Location.ToString())
		if (cursorLocation.Z >= pathData.Location.Z) break;
	}
	Cast<AThrowWeaponBase>(GetCurrentWeaponRef())->UpdateProjectilePathSpline(pathPointList);
}

FVector AMainCharacterBase::GetThrowDestination()
{
	FVector cursorWorldLocation = GetController<AMainCharacterController>()->GetCursorDeprojectionWorldLocation();
	if (cursorWorldLocation == FVector(0.0f)) return FVector(0.0f);

	//커서와 손 위치의 Z값을 일치시킴
	FVector startLocation = GetMesh()->GetSocketLocation("weapon_r");
	startLocation = { startLocation.X, startLocation.Y, cursorWorldLocation.Z };

	//그 상태에서 거리를 재서 최대 거리를 벗어나지 않는다면 그대로 커서 위치 반환
	if (UKismetMathLibrary::Vector_Distance(startLocation, cursorWorldLocation) < MAX_THROW_DISTANCE)
		return cursorWorldLocation;

	//그렇지 않다면, 최대 거리 만큼 제한하여 반환
	startLocation += UKismetMathLibrary::Normal(cursorWorldLocation - startLocation) * MAX_THROW_DISTANCE;
	return startLocation;
}

void AMainCharacterBase::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddMovementInput({ 1.0f, 0.0f, 0.0f }, MovementVector.Y);
		AddMovementInput({ 0.0f, 1.0f, 0.0f }, MovementVector.X);
	}
}

void AMainCharacterBase::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMainCharacterBase::Sprint(const FInputActionValue& Value)
{
	if (CharacterState.bIsSprinting) return;
	if (GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero()) return;

	CharacterState.bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = CharacterStat.DefaultMoveSpeed;
}

void AMainCharacterBase::StopSprint(const FInputActionValue& Value)
{
	if (!CharacterState.bIsSprinting) return;

	CharacterState.bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = CharacterStat.DefaultMoveSpeed / 2.0f;
}

void AMainCharacterBase::Roll()
{	
	if (!GetIsActionActive(ECharacterActionType::E_Idle)) return;
	
	FVector newDirection(0.0f);
	FRotator newRotation = FRotator();
	newDirection.X = GetInputAxisValue(FName("MoveForward"));
	newDirection.Y = GetInputAxisValue(FName("MoveRight"));

	//아무런 방향 입력이 없다면? 
	if (newDirection.Y + newDirection.X == 0)
	{
		//메시의 방향으로 구르기
		newDirection = GetMesh()->GetComponentRotation().Vector();
		newRotation = { 0, FMath::Atan2(newDirection.Y, newDirection.X) * 180.0f / 3.141592, 0.0f };
	}
	else //아니라면, 입력 방향으로 구르기
	{
		newRotation = { 0, FMath::Atan2(newDirection.Y, newDirection.X) * 180.0f / 3.141592, 0.0f };
		newRotation.Yaw += 270.0f;
	}
	
	//Rotation 리셋 로직
	GetWorldTimerManager().ClearTimer(RotationResetTimerHandle);
	ResetRotationToMovement();
	SetActorRotation(newRotation + FRotator(0.0f, 90.0f, 0.0f));
	GetMesh()->SetWorldRotation(newRotation);

	// 사운드
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "Roll");
	}

	//애니메이션 
	CharacterState.CharacterActionState = ECharacterActionType::E_Roll;
	if (AnimAssetData.RollAnimMontageList.Num() > 0
		&& IsValid(AnimAssetData.RollAnimMontageList[0]))
	{
		PlayAnimMontage(AnimAssetData.RollAnimMontageList[0], FMath::Max(1.0f, CharacterStat.DefaultMoveSpeed / 500.0f));
	}

	if (OnRoll.IsBound())
		OnRoll.Broadcast();
}

void AMainCharacterBase::Dash()
{
	if (IsActorBeingDestroyed()) return;
	LaunchCharacter(FVector(0.0f, 0.0f, 500.0f), false, false);
	GetWorldTimerManager().SetTimer(DashDelayTimerHandle, this, &AMainCharacterBase::StopDashMovement, 0.075f, false);
}

void AMainCharacterBase::ZoomIn(const FInputActionValue& Value)
{
	float value = Value.Get<float>();
	if (value == 0.0f) return;
	float newLength = CameraBoom->TargetArmLength;
	newLength = newLength + value * 75.0f;
	newLength = newLength < MIN_CAMERA_BOOM_LENGTH ? MIN_CAMERA_BOOM_LENGTH : newLength;
	newLength = newLength > MAX_CAMERA_BOOM_LENGTH ? MAX_CAMERA_BOOM_LENGTH : newLength;
	CameraBoom->TargetArmLength = newLength;
	if (OnZoom.IsBound())
		OnZoom.Broadcast();
}

void AMainCharacterBase::TryInvestigate()
{
	TArray<AActor*> nearActorList = GetNearInteractionActorList();

	if (nearActorList.Num())
	{
		if (AnimAssetData.InvestigateAnimMontageList.Num() > 0
			&& IsValid(AnimAssetData.InvestigateAnimMontageList[0]))
		{
			PlayAnimMontage(AnimAssetData.InvestigateAnimMontageList[0]);
		}
		Investigate(nearActorList[0]);
		ResetActionState();
	}
}

void AMainCharacterBase::Investigate(AActor* TargetActor)
{
	if (!IsValid(TargetActor)) return;

	if (TargetActor->ActorHasTag("Item"))
	{
		Cast<AItemBase>(TargetActor)->OnPlayerBeginPickUp.ExecuteIfBound(this);
		if (AssetManagerBaseRef.IsValid())
		{
			AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "EquipWeapon");
		}
	}
	else if (TargetActor->ActorHasTag("ItemBox"))
	{
		Cast<AItemBoxBase>(TargetActor)->OnPlayerOpenBegin.Broadcast(this);
		if (AssetManagerBaseRef.IsValid())
		{
			AssetManagerBaseRef.Get()->PlayRandomSound(this, ESoundAssetType::E_Character, 0, "OpenItemBox");
		}
	}
	else if (TargetActor->ActorHasTag("RewardBox"))
	{
		Cast<ARewardBoxBase>(TargetActor)->OnPlayerBeginInteract.Broadcast(this);
		if (AssetManagerBaseRef.IsValid())
		{
			AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "OpenAbilityBox");
		}
	}
	else if (TargetActor->ActorHasTag("CraftingBox"))
	{
		Cast<ACraftBoxBase>(TargetActor)->OnPlayerOpenBegin.Broadcast(this);
	}
	else if (TargetActor->ActorHasTag("Gate"))
	{
		Cast<AGateBase>(TargetActor)->EnterGate(); 
	}
}

void AMainCharacterBase::TryReload()
{
	Super::TryReload();
}

float AMainCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	//Disable this Logic until Facial Material Logic usable
	/*if (damageAmount > 0.0f && DamageCauser->ActorHasTag("Enemy"))
	{
		SetFacialDamageEffect();
		GetWorld()->GetTimerManager().ClearTimer(FacialEffectResetTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(FacialEffectResetTimerHandle, this, &AMainCharacterBase::ResetFacialDamageEffect, 1.0f, false);
	}*/
	return damageAmount;
}

void AMainCharacterBase::TryAttack()
{
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Attack
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (GetCurrentWeaponRef()->GetWeaponType() == EWeaponType::E_Throw) return;

	//IndieGo용 임시 코드----------------------------------------------------------
	if(GetCurrentWeaponRef()->GetWeaponStat().WeaponID == 12130) return;
	//---------------------------------------------------------------------------------

	if (GetCurrentWeaponRef()->WeaponID == 0)
	{
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("무기가 없습니다.")), FColor::White);
		return;
	}

	//=============

	//공격을 하고, 커서 위치로 Rotation을 조정
	this->Tags.Add("Attack|Common");
	//RotateToCursor();
	Super::TryAttack();

	GetWorldTimerManager().ClearTimer(AttackLoopTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(AttackLoopTimerHandle, FTimerDelegate::CreateLambda([&]() {
		if (!IsActorBeingDestroyed() && PlayerControllerRef.Get()->GetActionKeyIsDown("Attack"))
			TryAttack(); //0.1초 뒤에 체크해서 계속 눌려있는 상태라면, Attack을 반복한다. 
	}), 0.1f, false);
}

void AMainCharacterBase::TrySkill(ESkillType SkillType, int32 SkillID)
{
	if (!IsValid(GetCurrentWeaponRef()) || GetCurrentWeaponRef()->IsActorBeingDestroyed()) return;
	
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Skill
		|| CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	//IndieGo용 임시 코드----------------------------------------------------------
	if (GetCurrentWeaponRef()->GetWeaponStat().WeaponID == 12130)
	{
		CharacterState.CharacterCurrSkillGauge = 10;
	}
	//---------------------------------------------------------------------------------

	Super::TrySkill(SkillType, SkillID);

	//Try Skill and adjust rotation to cursor position
	RotateToCursor();
}

void AMainCharacterBase::AddSkillGauge()
{
	if (!IsValid(GetCurrentWeaponRef()) || GetCurrentWeaponRef()->IsActorBeingDestroyed()) return;

	AWeaponBase* weaponRef = GetCurrentWeaponRef();
	CharacterState.CharacterCurrSkillGauge += weaponRef->GetWeaponStat().SkillGaugeAug;
}


void AMainCharacterBase::Attack()
{
	Super::Attack();
	// 공격 델리게이트 호출
	if (OnAttack.IsBound())
		OnAttack.Broadcast();
}


void AMainCharacterBase::StopAttack()
{
	Super::StopAttack();
	if (IsValid(GetCurrentWeaponRef()))
	{
		GetCurrentWeaponRef()->StopAttack();
	}
}

void AMainCharacterBase::Die()
{
	Super::Die();
	if (GamemodeRef.IsValid())
	{
		if(IsValid(GamemodeRef.Get()->GetChapterManagerRef())
			&& IsValid(GamemodeRef.Get()->GetChapterManagerRef()->GetCurrentStage()))
			GamemodeRef.Get()->GetChapterManagerRef()->GetCurrentStage()->AIOffDelegate.Broadcast();
		
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
		//ClearAllTimerHandle();
		GamemodeRef.Get()->ClearResourceDelegate.Broadcast();
		GamemodeRef.Get()->FinishChapterDelegate.Broadcast(true);
	}
}

void AMainCharacterBase::RotateToCursor()
{
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Attack) return;
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Throw) return;

	FRotator newRotation = PlayerControllerRef.Get()->GetRotationToCursor();
	if (newRotation != FRotator())
	{
		newRotation.Pitch = newRotation.Roll = 0.0f;
		GetMesh()->SetWorldRotation(newRotation);
	}
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetMesh()->SetWorldRotation(newRotation.Quaternion());

	GetWorld()->GetTimerManager().ClearTimer(RotationResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(RotationResetTimerHandle, FTimerDelegate::CreateLambda([&]() {
		ResetRotationToMovement();
		FRotator newRotation = PlayerControllerRef.Get()->GetLastRotationToCursor();
		newRotation.Yaw = FMath::Fmod((newRotation.Yaw + 90.0f), 360.0f);
		SetActorRotation(newRotation.Quaternion(), ETeleportType::ResetPhysics);
	}), 1.0f, false);
}

void AMainCharacterBase::PickSubWeapon(const FInputActionValue& Value)
{
	FVector typeVector = Value.Get<FVector>();
	int32 targetIdx = typeVector.X - 1;

	//If player press current picked sub weapon, switch to the first main weapon.
	if (GetCurrentWeaponRef()->GetWeaponType() == EWeaponType::E_Throw && GetSubInventoryRef()->GetCurrentIdx() == targetIdx)
	{
		GetInventoryRef()->EquipWeaponByIdx(0);
		GetSubInventoryRef()->OnInventoryItemIsUpdated.Broadcast(-1, false, FInventoryItemInfoStruct());
		return;
	}

	//Try switch to subweapon
	bool result = TrySwitchToSubWeapon(targetIdx);
	if (result)
	{
		//Force UI update with manual calling delegate
		GetInventoryRef()->OnInventoryItemIsUpdated.Broadcast(-1, false, FInventoryItemInfoStruct());
	}
	else
	{
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("무기가 없습니다.")), FColor::White);
	}
}

TArray<AActor*> AMainCharacterBase::GetNearInteractionActorList()
{
	TArray<AActor*> totalItemList;
	TArray<UClass*> targetClassList = {AItemBase::StaticClass(), AItemBoxBase::StaticClass()
									 , ARewardBoxBase::StaticClass(), ACraftBoxBase::StaticClass()
									 , AGateBase::StaticClass()};
	TEnumAsByte<EObjectTypeQuery> itemObjectType = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	FVector overlapBeginPos = GetActorLocation() + GetMesh()->GetForwardVector() * 70.0f + GetMesh()->GetUpVector() * -45.0f;
	
	for (float sphereRadius = 0.2f; sphereRadius < 15.0f; sphereRadius += 0.2f)
	{
		bool result = false;

		for (UClass* targetClass : targetClassList)
		{
			result = (result || UKismetSystemLibrary::SphereOverlapActors(GetWorld(), overlapBeginPos, sphereRadius
													, { itemObjectType }, targetClass, {}, totalItemList));
			if (totalItemList.Num() > 0) return totalItemList; //찾는 즉시 반환
		}
	}
	return totalItemList;
}

void AMainCharacterBase::StopDashMovement()
{
	const FVector& direction = GetMesh()->GetRightVector();
	float& speed = GetCharacterMovement()->MaxWalkSpeed;
	GetCharacterMovement()->Velocity = direction * (speed + 1000.0f);
}

void AMainCharacterBase::SetCharacterStatFromSaveData()
{
	CharacterStat = SavedData.PlayerSaveGameData.PlayerStat;
}

void AMainCharacterBase::ResetRotationToMovement()
{
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Roll) return;
	FRotator newRotation = GetCapsuleComponent()->GetComponentRotation();
	newRotation.Yaw += 270.0f;
	GetMesh()->SetWorldRotation(newRotation);
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AMainCharacterBase::SwitchToNextWeapon()
{
	//Block switching weapon in particular actions
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Skill ||
		CharacterState.CharacterActionState == ECharacterActionType::E_Attack ||
		CharacterState.CharacterActionState == ECharacterActionType::E_Throw ||
		CharacterState.CharacterActionState == ECharacterActionType::E_Reload) return;

	if (GetCurrentWeaponRef()->GetWeaponType() == EWeaponType::E_Throw)
	{
		GetInventoryRef()->EquipWeaponByIdx(0);
		GetSubInventoryRef()->OnInventoryItemIsUpdated.Broadcast(-1, false, FInventoryItemInfoStruct());
		return;
	}

	Super::SwitchToNextWeapon();
}

void AMainCharacterBase::DropWeapon()
{
	Super::DropWeapon();
}

bool AMainCharacterBase::TryAddNewDebuff(FDebuffInfoStruct DebuffInfo, AActor* Causer)
{
	if (!Super::TryAddNewDebuff(DebuffInfo, Causer)) return false;

	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "Debuff");
	}
	//230621 임시 제거
	//ActivateBuffNiagara(bIsDebuff, BuffDebuffType);

	//GetWorld()->GetTimerManager().ClearTimer(BuffEffectResetTimerHandle);
	//BuffEffectResetTimerHandle.Invalidate();
	//GetWorld()->GetTimerManager().SetTimer(BuffEffectResetTimerHandle, FTimerDelegate::CreateLambda([&]() {
	//	DeactivateBuffEffect();
	//}), DebuffInfo.TotalTime, false);

	return true;
}

bool AMainCharacterBase::TryAddNewAbility(const int32 NewAbilityID)
{
	if(!IsValid(AbilityManagerRef)) return false;
	return AbilityManagerRef->TryAddNewAbility(NewAbilityID);
}

bool AMainCharacterBase::TryRemoveAbility(const int32 NewAbilityID)
{
	if (!IsValid(AbilityManagerRef)) return false;
	return AbilityManagerRef->TryRemoveAbility(NewAbilityID);
}

bool AMainCharacterBase::GetIsAbilityActive(const int32 AbilityID)
{
	if (!IsValid(AbilityManagerRef)) return false;
	return AbilityManagerRef->GetIsAbilityActive(AbilityID);
}

bool AMainCharacterBase::PickWeapon(const int32 NewWeaponID)
{
	if (!IsValid(GetInventoryRef()) || !IsValid(GetSubInventoryRef())) return false;
	
	bool result = false; 
	EWeaponType weaponType = GetInventoryRef()->GetWeaponType(NewWeaponID);
	
	if (weaponType == EWeaponType::E_Throw)
	{
		result = GetSubInventoryRef()->AddWeapon(NewWeaponID);
	}
	else if(weaponType != EWeaponType::E_None)
	{
		result = GetInventoryRef()->AddWeapon(NewWeaponID);
	}

	return result;
}

void AMainCharacterBase::ActivateDebuffNiagara(uint8 DebuffType)
{
	TArray<UNiagaraSystem*>* targetEmitterList = &DebuffNiagaraEffectList;

	if (targetEmitterList->IsValidIndex(DebuffType) && (*targetEmitterList)[DebuffType] != nullptr)
	{
		BuffNiagaraEmitter->SetRelativeLocation(FVector(0.0f, 0.0f, 125.0f));
		BuffNiagaraEmitter->Deactivate();
		BuffNiagaraEmitter->SetAsset((*targetEmitterList)[DebuffType], false);
		BuffNiagaraEmitter->Activate();
	}
}

void AMainCharacterBase::DeactivateBuffEffect()
{
	BuffNiagaraEmitter->SetAsset(nullptr, false);
	BuffNiagaraEmitter->Deactivate(); 
}

void AMainCharacterBase::SetFacialDamageEffect()
{
	UMaterialInstanceDynamic* currMaterial = CurrentDynamicMaterialList[1];
	
	if (currMaterial != nullptr && EmotionTextureList.Num() >= 3)
	{
		currMaterial->SetTextureParameterValue(FName("BaseTexture"), EmotionTextureList[(uint8)(EEmotionType::E_Angry)]);
		currMaterial->SetScalarParameterValue(FName("bIsDamaged"), true);
	}
}

void AMainCharacterBase::ResetFacialDamageEffect()
{
	UMaterialInstanceDynamic* currMaterial = CurrentDynamicMaterialList[1];

	if (currMaterial != nullptr && EmotionTextureList.Num() >= 3)
	{
		currMaterial->SetTextureParameterValue(FName("BaseTexture"), EmotionTextureList[(uint8)(EEmotionType::E_Idle)]);
		currMaterial->SetScalarParameterValue(FName("bIsDamaged"), false);
	}
}

void AMainCharacterBase::ClearAllTimerHandle()
{
	Super::ClearAllTimerHandle();

	GetWorldTimerManager().ClearTimer(BuffEffectResetTimerHandle);
	GetWorldTimerManager().ClearTimer(FacialEffectResetTimerHandle);
	GetWorldTimerManager().ClearTimer(RollTimerHandle); 
	GetWorldTimerManager().ClearTimer(AttackLoopTimerHandle);
	GetWorldTimerManager().ClearTimer(DashDelayTimerHandle);

	BuffEffectResetTimerHandle.Invalidate();
	FacialEffectResetTimerHandle.Invalidate();
	RollTimerHandle.Invalidate();
	AttackLoopTimerHandle.Invalidate();
	DashDelayTimerHandle.Invalidate();
}
