﻿// Fill out your copyright notice in the Description page of Project Settings.
#include "MainCharacterBase.h"
#include "MainCharacterController.h"
#include "../Component/TargetingManagerComponent.h"
#include "../Component/ItemInventoryComponent.h"
#include "../Component/SkillManagerComponentBase.h"
#include "../Component/PlayerSkillManagerComponent.h"
#include "../Component/ActionTrackingComponent.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SaveSystem/BackStreetGameInstance.h"
#include "../../System/AbilitySystem/AbilityManagerBase.h"
#include "../../System/CraftingSystem/CraftBoxBase.h"
#include "../../System/AssetSystem/AssetManagerBase.h"
#include "../../Item/ItemBase.h"
#include "../../Item/ItemBoxBase.h"
#include "../../Item/RewardBoxBase.h"
#include "../../Item/Weapon/Ranged/RangedCombatManager.h"
#include "../../Item/InteractiveCollisionComponent.h"
#include "../../System/MapSystem/Stage/GateBase.h"
#include "../Component/WeaponComponentBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AudioComponent.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimMontage.h"
#define MAX_CAMERA_BOOM_LENGTH 1450.0f
#define MIN_CAMERA_BOOM_LENGTH 250.0f

// Sets default values
AMainCharacterBase::AMainCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//SetActorTickInterval(0.1f);

	//MainCharacter Main Camera SpringArm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CAMERA_BOOM"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 375.0f;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;	
	CameraBoom->CameraRotationLagSpeed = 0.5f;
	CameraBoom->CameraLagMaxDistance = 1000.0f;
	CameraBoom->SetRelativeLocation({ 0.0f, 30.0f, 25.0f });
	CameraBoom->SetWorldRotation({ -45.0f, 0.0f, 0.0f });


	//MainCharacter RangedWeaponAim SpringArm
	RangedAimBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("RangedWeaponAim_Boom"));
	RangedAimBoom->SetupAttachment(RootComponent);				
	RangedAimBoom->TargetArmLength = 150.0f;					
	RangedAimBoom->bUsePawnControlRotation = true;
	RangedAimBoom->bInheritPitch = true;
	RangedAimBoom->bInheritRoll = false;
	RangedAimBoom->bInheritYaw = true;
	RangedAimBoom->SetRelativeLocation({ 0.0f, 80.0f, -10 });	//Set Location

	HitSceneComponent->SetRelativeLocation(FVector(0.0f, 110.0f, 120.0f));

	//MainCharacter Main Camera
	FollowingCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FOLLOWING_CAMERA"));
	FollowingCamera->SetupAttachment(CameraBoom);
	FollowingCamera->bAutoActivate = true;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SOUND"));
	ItemInventory = CreateDefaultSubobject<UItemInventoryComponent>(TEXT("Item_Inventory"));

	SkillManagerComponent = CreateDefaultSubobject<UPlayerSkillManagerComponent>(TEXT("SKILL_MANAGER_"));
	SkillManagerComponentRef = SkillManagerComponent;

	AbilityManagerComponent = CreateDefaultSubobject<UAbilityManagerComponent>(TEXT("ABILITY_MANAGER"));

	GetCapsuleComponent()->OnComponentHit.AddUniqueDynamic(this, &AMainCharacterBase::OnCapsuleHit);
	GetCapsuleComponent()->SetCapsuleRadius(41.0f);

	this->bUseControllerRotationYaw = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->JumpZVelocity = 1000.0f;
	GetCharacterMovement()->RotationRate = { 0.0f, 1000.0f, 0.0f };
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->GravityScale = 2.5f;

	this->Tags.Add("Player");
}

// Called when the game starts or when spawned
void AMainCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitCharacterGameplayInfo(DefaultStat);
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	PlayerControllerRef = Cast<AMainCharacterController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	AbilityManagerComponent->InitAbilityManager(this);
	InitCombatUI();


	//UBackStreetGameInstance* gameInstance = Cast<UBackStreetGameInstance>(GetGameInstance());

	//Load SaveData
	//if (gameInstance->LoadGameSaveData(SavedData)) return;
	//else
	//{
	//	gameInstance->SaveGameData(FSaveData());
	//}
	//SetCharacterStatFromSaveData();
	//ItemInventory->SetItemInventoryFromSaveData();
	ItemInventory->InitInventory();
	
	TargetingManagerComponent->OnTargetingActivated.AddDynamic(this, &AMainCharacterBase::OnTargetingStateUpdated);
	SetAutomaticRotateModeTimer();
}

// Called every frame
void AMainCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UpdateWallThroughEffect();
}

void AMainCharacterBase::InitAsset(int32 NewCharacterID)
{
	Super::InitAsset(NewCharacterID);
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
		EnhancedInputComponent->BindAction(InputActionInfo.MoveAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Move);
		EnhancedInputComponent->BindAction(InputActionInfo.MoveAction, ETriggerEvent::Completed, this, &AMainCharacterBase::ResetMovementInputValue);

		//Look 
		EnhancedInputComponent->BindAction(InputActionInfo.LookAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Look);

		//Rolling
		EnhancedInputComponent->BindAction(InputActionInfo.RollAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Roll);

		//Attack
		EnhancedInputComponent->BindAction(InputActionInfo.AttackAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryAttack);

		//Upper Attack
		//EnhancedInputComponent->BindAction(InputActionInfo.UpperAttackAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryUpperAttack);

		//Sprint
		EnhancedInputComponent->BindAction(InputActionInfo.SprintAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Sprint);
		EnhancedInputComponent->BindAction(InputActionInfo.SprintAction, ETriggerEvent::Completed, this, &AMainCharacterBase::StopSprint);

		//Jump
		EnhancedInputComponent->BindAction(InputActionInfo.JumpAction, ETriggerEvent::Completed, this, &AMainCharacterBase::StartJump);

		//Zoom In&Out
		EnhancedInputComponent->BindAction(InputActionInfo.ZoomAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::ZoomIn);
		EnhancedInputComponent->BindAction(InputActionInfo.ZoomAction, ETriggerEvent::Completed, this, &AMainCharacterBase::ZoomOut);

		//Interaction
		EnhancedInputComponent->BindAction(InputActionInfo.InvestigateAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryInvestigate);

		//SubWeapon
		EnhancedInputComponent->BindAction(InputActionInfo.ShootAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryShoot);

		//EnhancedInputComponent->BindAction(LockToTargetAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::LockToTarget);
	}
}

void AMainCharacterBase::SwitchWeapon(bool bSwitchToSubWeapon)
{
	if (!IsValid(ItemInventory) || (!GetIsActionActive(ECharacterActionType::E_Idle) && !GetIsActionActive(ECharacterActionType::E_Shoot))) return;
	FItemInfoDataStruct subWeaponData = ItemInventory->GetSubWeaponInfoData();
	FItemInfoDataStruct mainWeaponData = ItemInventory->GetMainWeaponInfoData();

	if (bSwitchToSubWeapon && WeaponComponent->WeaponStat.WeaponType == EWeaponType::E_Melee)
	{
		if (subWeaponData.ItemID == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("SwitchWeapon %d Not Found"), subWeaponData.ItemID);
			return;
		}
		WeaponComponent->InitWeapon(subWeaponData.ItemID - 20000); //temp code
	}
	else if (!bSwitchToSubWeapon && WeaponComponent->WeaponStat.WeaponType == EWeaponType::E_Shoot)
	{
		if (mainWeaponData.ItemID == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("SwitchWeapon %d Not Found"), mainWeaponData.ItemID);
			return;
		}
		WeaponComponent->InitWeapon(mainWeaponData.ItemID - 20000); //temp code
	}
}

void AMainCharacterBase::ZoomIn()
{
	//Exception Handling ==========================
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (!IsValid(ItemInventory)) return;
	FItemInfoDataStruct subWeaponData = ItemInventory->GetSubWeaponInfoData();
	FItemInfoDataStruct mainWeaponData = ItemInventory->GetMainWeaponInfoData();
	if (subWeaponData.ItemID == 0) return;

	SwitchWeapon(true);										
	SetAimingMode(true);

	//FollowingCamera attach to RangedAimBoom Component using Interp ==================
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	FollowingCamera->AttachToComponent(RangedAimBoom, FAttachmentTransformRules::KeepWorldTransform);
	UKismetSystemLibrary::MoveComponentTo(FollowingCamera, FVector(0, 0, 0), FRotator(0, 0, 0)
		, true, true, 0.2, false, EMoveComponentAction::Type::Move, LatentInfo);

	OnZoomBegin.Broadcast();
}

void AMainCharacterBase::ZoomOut()
{
	if (WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Shoot) return;

	SwitchWeapon(false);
	SetAimingMode(false);
	
	//FollowingCamera attach to CameraBoom Component using Interp ===================
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	FollowingCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepWorldTransform);
	UKismetSystemLibrary::MoveComponentTo(FollowingCamera, FVector(0, 0, 0), FRotator(0, 0, 0)
		, true, true, 0.2, false, EMoveComponentAction::Type::Move, LatentInfo);

	OnZoomEnd.Broadcast();
}

void AMainCharacterBase::TryShoot()
{
	if (WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Shoot) return;
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Shoot) return;

	FRotator shootRotation = GetAimingRotation(WeaponComponent->GetComponentLocation());
	WeaponComponent->RangedCombatManager->TryFireProjectile(shootRotation);

	if (AssetHardPtrInfo.ShootAnimMontageList[0] > 0
		&& IsValid(AssetHardPtrInfo.ShootAnimMontageList[0]))
	{
		PlayAnimMontage(AssetHardPtrInfo.ShootAnimMontageList[0], 1.0f);
		OnAttackStarted.Broadcast();
	}
}

void AMainCharacterBase::SetAimingMode(bool bNewState)
{
	GetCharacterMovement()->bOrientRotationToMovement = !bNewState;
	bUseControllerRotationYaw = bNewState;
	CharacterGameplayInfo.CharacterActionState = bNewState ? ECharacterActionType::E_Shoot : ECharacterActionType::E_Idle;
	CharacterGameplayInfo.bIsAiming = bNewState;
	CharacterGameplayInfo.bIsSprinting = bNewState ? false : CharacterGameplayInfo.bIsSprinting;

	const float moveSpeed = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed);
	const float aimMoveSpeed = bNewState ? moveSpeed * 0.3f : moveSpeed;
	SetWalkSpeedWithInterp(aimMoveSpeed, 0.75f);
}

FRotator AMainCharacterBase::GetAimingRotation(FVector BeginLocation)
{
	FHitResult lineTraceHitResult; //LineTracing의 결과가 담길 변수
	FVector traceBeginLocation = FollowingCamera->GetComponentLocation(); //Trace는 카메라에서 시작
	FVector traceEndLocation = traceBeginLocation + (FollowingCamera->GetForwardVector()) * 200000.0f; //End는 Camera로부터 20000.0f 떨어진 지점까지
	FCollisionQueryParams traceCollisionQuery = FCollisionQueryParams::DefaultQueryParam;
	traceCollisionQuery.AddIgnoredActor(this->GetUniqueID());  //플레이어의 카메라가 Hit되지 않도록 방지

	GetWorld()->LineTraceSingleByChannel(lineTraceHitResult, traceBeginLocation, traceEndLocation
		, ECollisionChannel::ECC_Camera, traceCollisionQuery); //LineTrace 시작
	
	//trace가 맞지 않을 경우 trace의 끝 지점을 rotation 연산의 끝 지점으로 지정
	FVector targetLocation = lineTraceHitResult.bBlockingHit ? lineTraceHitResult.ImpactPoint : traceEndLocation;
	FRotator beginRotation = UKismetMathLibrary::FindLookAtRotation(BeginLocation, targetLocation);
	return beginRotation;
}

void AMainCharacterBase::ResetMovementInputValue()
{
	MovementInputValue = FVector2D::ZeroVector;
}

void AMainCharacterBase::Move(const FInputActionValue& Value)
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (GetCharacterMovement()->IsFalling()) return;
	if (!ActionTrackingComponent->GetIsActionReady(FName("JumpAttack"))) return;
	if (CharacterGameplayInfo.bIsAirAttacking || CharacterGameplayInfo.bIsDownwardAttacking) return;

	// input is a Vector2D
	MovementInputValue = Value.Get<FVector2D>();
	if (MovementInputValue == FVector2D::ZeroVector) return;

	if (Controller != nullptr)
	{
		FVector forwardAxis = FollowingCamera->GetForwardVector();
		//!CharacterState.TargetedEnemy.IsValid() ? FollowingCamera->GetForwardVector()
		//					  : CharacterState.TargetedEnemy.Get()->GetActorLocation() - GetActorLocation();
		FVector rightAxis = UKismetMathLibrary::RotateAngleAxis(forwardAxis, 90.0f, GetActorUpVector());

		rightAxis.Z = forwardAxis.Z = 0.0f;

		AddMovementInput(forwardAxis, MovementInputValue.Y);
		AddMovementInput(FollowingCamera->GetRightVector(), MovementInputValue.X);

		OnMoveStarted.Broadcast();
		SetRotationLagSpeed(Value.Get<FVector2D>());
	}
}

void AMainCharacterBase::Look(const FInputActionValue& Value)
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr && !TargetingManagerComponent->GetIsTargetingActivated())
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X / 2.0f);
		AddControllerPitchInput(LookAxisVector.Y / 2.0f);
		SetManualRotateMode();

		// set timer for automatic rotate mode 
		if (LookAxisVector.Length() <= 0.25f)
		{
			SetAutomaticRotateModeTimer();
		}
	}
}

void AMainCharacterBase::StartJump(const FInputActionValue& Value)
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (!ActionTrackingComponent->GetIsActionReady(FName("JumpAttack"))) return;

	Jump();
	OnJumpStarted.Broadcast();
}

void AMainCharacterBase::Sprint(const FInputActionValue& Value)
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (CharacterGameplayInfo.bIsSprinting) return;
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero()) return;
	
	WeaponComponent->ResetComboCnt();
	CharacterGameplayInfo.bIsSprinting = true;
	SetWalkSpeedWithInterp(CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed) * 1.25, 0.75f);
	SetFieldOfViewWithInterp(105.0f, 0.25f);
	OnSprintStarted.Broadcast();
}

void AMainCharacterBase::StopSprint(const FInputActionValue& Value)
{
	if (!CharacterGameplayInfo.bIsSprinting) return;

	CharacterGameplayInfo.bIsSprinting = false;
	SetWalkSpeedWithInterp(CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed), 0.4f);
	SetFieldOfViewWithInterp(90.0f, 0.5f);
	OnSprintEnd.Broadcast();
}

void AMainCharacterBase::Roll()
{	
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (!GetIsActionActive(ECharacterActionType::E_Idle) && !GetIsActionActive(ECharacterActionType::E_Attack)) return;
	if (!ActionTrackingComponent->GetIsActionReady(FName("JumpAttack"))) return;
	if (!CharacterGameplayInfo.bCanRoll || CharacterGameplayInfo.bIsAirAttacking || CharacterGameplayInfo.bIsDownwardAttacking) return;

	if (GetIsActionActive(ECharacterActionType::E_Attack))
	{
		StopAttack(); 
		ResetActionState();
	}
	
	FVector newDirection(0.0f);
	FRotator newRotation = FollowingCamera->GetComponentRotation();
	newRotation.Pitch = newRotation.Roll = 0.0f;
	newDirection.X = MovementInputValue.Y;
	newDirection.Y = MovementInputValue.X;

	//아무런 방향 입력이 없다면? 
	if (newDirection.Y + newDirection.X == 0)
	{
		//메시의 방향으로 구르기
		newDirection = GetMesh()->GetComponentRotation().Vector();
		newRotation = { 0, FMath::Atan2(newDirection.Y, newDirection.X) * 180.0f / 3.141592, 0.0f };
	}
	else //아니라면, 입력 방향으로 구르기
	{
		float targetYawValue = FMath::Atan2(newDirection.Y, newDirection.X) * 180.0f / 3.141592;
		targetYawValue += 270.0f;
		newRotation.Yaw = FMath::Fmod(newRotation.Yaw + targetYawValue, 360.0f);
	}
	
	// 시점 전환을 위해 제거
	//Rotation 리셋 로직
	GetWorldTimerManager().ClearTimer(RotationResetTimerHandle);
	//ResetRotationToMovement();
	SetActorRotation(newRotation + FRotator(0.0f, 90.0f, 0.0f));
	GetMesh()->SetWorldRotation(newRotation);

	// 사운드
	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "Roll");
	}

	//애니메이션 
	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Roll;
	if (AssetHardPtrInfo.RollAnimMontageList.Num() > 0
		&& IsValid(AssetHardPtrInfo.RollAnimMontageList[0]))
	{
		PlayAnimMontage(AssetHardPtrInfo.RollAnimMontageList[0], 1.0f);
		OnRollStarted.Broadcast();
		
		CharacterGameplayInfo.bCanRoll = false;
		GetWorldTimerManager().SetTimer(RollDelayTimerHandle, FTimerDelegate::CreateLambda([=]() {
			CharacterGameplayInfo.bCanRoll = true;
		}), CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_RollDelay), false);
	}
}

void AMainCharacterBase::Dash()
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (IsActorBeingDestroyed()) return;
	LaunchCharacter(FVector(0.0f, 0.0f, 500.0f), false, false);
	GetWorldTimerManager().SetTimer(DashDelayTimerHandle, this, &AMainCharacterBase::StopDashMovement, 0.075f, false);
}

void AMainCharacterBase::TryInvestigate()
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	TArray<UInteractiveCollisionComponent*> nearCompoenentList = GetNearInteractionComponentList();

	if (nearCompoenentList.Num())
	{
		if (AssetHardPtrInfo.InvestigateAnimMontageList.Num() > 0
			&& IsValid(AssetHardPtrInfo.InvestigateAnimMontageList[0]))
		{
			PlayAnimMontage(AssetHardPtrInfo.InvestigateAnimMontageList[0]);
		}
		nearCompoenentList[0]->Interact();
		ResetActionState();
	}
}

void AMainCharacterBase::LockToTarget(const FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (CharacterGameplayInfo.bIsAirAttacking || CharacterGameplayInfo.bIsDownwardAttacking) return;

	TargetingManagerComponent->ActivateTargeting();
}

void AMainCharacterBase::SnapToCharacter(AActor* Target)
{
	if (!IsValid(Target)) return;

	//@@@@@@@@ TEMPORARY CODE @@@@@@@@@@@@@@@@@@@@@@@
	if (WeaponComponent->GetCurrentComboCnt() == 0)
	{
		//if (CharacterGameplayInfo.CurrentSP < 0.5f) return;
		//CharacterGameplayInfo.CurrentSP -= 0.5f;
	}

	FVector startLocation = GetActorLocation();
	FVector endLocation = Target->GetActorLocation();
	FVector dirVector = endLocation - startLocation; dirVector.Normalize();
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(startLocation, endLocation));
	SetLocationWithInterp(startLocation + dirVector * (FVector::Distance(startLocation, endLocation) - 75.0f), 2.0f);
	if (GetDistanceTo(TargetingManagerComponent->GetTargetedCandidate()) > 400.0f)
	{
		SetFieldOfViewWithInterp(130.0f, 3.0f, true);
	}
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
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Attack
		&& CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle
		&& CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Shoot) return;
	if (!CharacterGameplayInfo.bCanAttack) return;
	if (WeaponComponent->WeaponID == 0)
	{
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("무기가 없습니다.")), FColor::White);
		return;
	}
	this->Tags.Add("Attack|Common");

	if (CharacterGameplayInfo.bIsSprinting && !CharacterGameplayInfo.bIsAirAttacking
		&& !CharacterGameplayInfo.bIsDownwardAttacking && !GetCharacterMovement()->IsFalling())
	{
		TryDashAttack();
		return;
	}

	//Rotate to attack direction using input (1. movement / 2. camera)
	if (IsValid(TargetingManagerComponent->GetTargetedCandidate())
		&& GetDistanceTo(TargetingManagerComponent->GetTargetedCandidate()) >= 200.0f
		&& !ActionTrackingComponent->GetIsActionInProgress("Attack")
		&& !ActionTrackingComponent->GetIsActionInProgress("DashAttack")
		&& !ActionTrackingComponent->GetIsActionInProgress("JumpAttack")
		&& WeaponComponent->GetWeaponStat().WeaponType == EWeaponType::E_Melee)
	{
		SnapToCharacter(TargetingManagerComponent->GetTargetedCandidate());
	}

	else if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Idle)
	{
		if (MovementInputValue.Length() > 0)
		{
			float turnAngle = FMath::RadiansToDegrees(FMath::Atan2(MovementInputValue.X, MovementInputValue.Y))
				+ FollowingCamera->GetComponentRotation().Yaw;
			SetActorRotation(FRotator(0.0f, turnAngle, 0.0f));
		}
		else if(ActionTrackingComponent->GetIsActionReady("Attack"))
		{
			SetActorRotation(FRotator(0.0f, FollowingCamera->GetComponentRotation().Yaw, 0.0f));
		}
	}

	Super::TryAttack();
}

void AMainCharacterBase::TryUpperAttack()
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (CharacterGameplayInfo.bIsAirAttacking) return;
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (CharacterGameplayInfo.bIsSprinting)
	{
		SetFieldOfViewWithInterp(90.0f, 0.75f);
	}
	AActor* targetedEnemy = TargetingManagerComponent->GetTargetedCharacter();

	//Update upper atk target enemy (cloest pawn)
	if (!IsValid(targetedEnemy)
		|| FVector::Distance(GetActorLocation(), targetedEnemy->GetActorLocation()) >= 300.0f)
	{
		TargetingManagerComponent->ForceTargetingToNearestCharacter();
		targetedEnemy = TargetingManagerComponent->GetTargetedCharacter();
	}
	
	if (IsValid(targetedEnemy) && !targetedEnemy->ActorHasTag("Boss")
		&& FVector::Distance(GetActorLocation(), targetedEnemy->GetActorLocation()) <= 300.0f)
	{
		FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(targetedEnemy->GetActorLocation(), GetActorLocation());
		targetedEnemy->SetActorRotation(newRotation);
		newRotation.Add(0.0f, 180.0f, 0.0f);
		SetActorRotation(newRotation);
		SetLocationWithInterp(Cast<ACharacterBase>(targetedEnemy)->HitSceneComponent->GetComponentLocation(), 100.0f);

		Super::TryUpperAttack();
	}
}

void AMainCharacterBase::TryDownwardAttack()
{
	Super::TryDownwardAttack();
	if (!GetCharacterMovement()->IsFalling() || !CharacterGameplayInfo.bIsAirAttacking) return;
	if (IsValid(TargetingManagerComponent->GetTargetedCharacter()))
	{
		SetFieldOfViewWithInterp(110.0f, 0.75f);
	}
}

bool AMainCharacterBase::TrySkill(int32 SkillID)
{	
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Attack)
	{
		StopAttack();
	}
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Skill
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Stun
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Die
		|| CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_KnockedDown) return false;

	return Super::TrySkill(SkillID);
}

void AMainCharacterBase::Attack()
{
	Super::Attack();
}


void AMainCharacterBase::StopAttack()
{
	Super::StopAttack();
	WeaponComponent->StopAttack();
}

void AMainCharacterBase::Die()
{
	Super::Die();
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void AMainCharacterBase::StandUp()
{
	Super::StandUp();
}

TArray<UInteractiveCollisionComponent*> AMainCharacterBase::GetNearInteractionComponentList()
{
	TArray<UInteractiveCollisionComponent*> returnList;
	TArray<UClass*> targetClassList = { UInteractiveCollisionComponent::StaticClass() };
	TEnumAsByte<EObjectTypeQuery> itemObjectType = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	FVector overlapBeginPos = GetActorLocation() + GetMesh()->GetForwardVector() * 70.0f + GetMesh()->GetUpVector() * -45.0f;
	
	for (float sphereRadius = 0.2f; sphereRadius < 15.0f; sphereRadius += 0.2f)
	{
		bool result = false;

		TArray<UPrimitiveComponent*> resultList;
		for (UClass* targetClass : targetClassList)
		{
			result = (result || UKismetSystemLibrary::SphereOverlapComponents(GetWorld(), overlapBeginPos, sphereRadius
						, { itemObjectType }, targetClass, {}, resultList));
			if (resultList.Num() > 0)
			{
				for (UPrimitiveComponent*& component : resultList)
					returnList.Add(Cast<UInteractiveCollisionComponent>(component));
				return returnList; //찾는 즉시 반환
			}
		}
	}
	return {};
}

void AMainCharacterBase::StopDashMovement()
{
	const FVector& direction = GetMesh()->GetRightVector();
	float& speed = GetCharacterMovement()->MaxWalkSpeed;
	GetCharacterMovement()->Velocity = direction * (speed + 1000.0f);
}

void AMainCharacterBase::SetCameraVerticalAlignmentWithInterp(float TargetPitch, const float InterpSpeed)
{
	FTimerDelegate updateFunctionDelegate;

	//Binding the function with specific values
	updateFunctionDelegate.BindUFunction(this, FName("UpdateCameraPitch"), TargetPitch, InterpSpeed);

	//Calling MyUsefulFunction after 5 seconds without looping
	GetWorld()->GetTimerManager().ClearTimer(CameraRotationAlignmentHandle);
	GetWorld()->GetTimerManager().SetTimer(CameraRotationAlignmentHandle, updateFunctionDelegate, 0.01f, true);
}

void AMainCharacterBase::ResetCameraRotation()
{
	SetManualRotateMode();
	FRotator newRotation = CameraBoom->GetRelativeRotation();
	newRotation.Yaw = newRotation.Roll = 0.0f;
	CameraBoom->SetRelativeRotation(newRotation);
}

void AMainCharacterBase::SetAutomaticRotateModeTimer()
{
	GetWorldTimerManager().ClearTimer(SwitchCameraRotateModeTimerHandle);
	SwitchCameraRotateModeTimerHandle.Invalidate();
	GetWorldTimerManager().SetTimer(SwitchCameraRotateModeTimerHandle, this, &AMainCharacterBase::SetAutomaticRotateMode, AutomaticModeSwitchTime);
}

void AMainCharacterBase::SetAutomaticRotateMode()
{
	if (TargetingManagerComponent->GetIsTargetingActivated()) return;
	
	SetCameraVerticalAlignmentWithInterp(GetControlRotation().Pitch <= 90.0f ? 0.0f : 360.0f, 0.25f);

	GetWorld()->GetTimerManager().SetTimer(RotationResetTimerHandle, FTimerDelegate::CreateLambda([&]()
		{
			CameraBoom->bEnableCameraRotationLag = true;
			CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
			CameraBoom->bUsePawnControlRotation = false;
			CameraBoom->bInheritYaw = true;
		}), 1.0f, false /*반복*/);
}

void AMainCharacterBase::UpdateCameraPitch(float TargetPitch, float InterpSpeed)
{
	FRotator currentRotation = GetControlRotation();
	if (FMath::IsNearlyEqual(currentRotation.Pitch, TargetPitch, 5.0f))
	{
		GetWorld()->GetTimerManager().ClearTimer(CameraRotationAlignmentHandle);
	}
	currentRotation.Pitch = FMath::FInterpTo(currentRotation.Pitch, TargetPitch, 0.1f, InterpSpeed);
	Controller->SetControlRotation(currentRotation);
}

void AMainCharacterBase::SetManualRotateMode()
{
	CameraBoom->bEnableCameraRotationLag = false;
	CameraBoom->bUsePawnControlRotation = true;
	if (Controller != nullptr)
	{
		GetController()->SetControlRotation(FollowingCamera->GetComponentRotation());
	}
	CameraBoom->bInheritYaw = true;
}

void AMainCharacterBase::SetRotationLagSpeed(FVector2D ModeInput)
{
	if (ModeInput.Y == -1.0f && FMath::Abs(ModeInput.X) <= 0.3f)
	{
		CameraBoom->CameraRotationLagSpeed = FaceToFaceLagSpeed;
	}
	else
	{
		CameraBoom->CameraRotationLagSpeed = NoramlLagSpeed;
	}
}

void AMainCharacterBase::OnTargetingStateUpdated(bool bIsActivated, APawn* Target)
{
	if (bIsActivated && IsValid(Target))
	{
		SetManualRotateMode();
	}
	else
	{
		SetAutomaticRotateMode();
	}
}

void AMainCharacterBase::SetCharacterStatFromSaveData()
{
	//CharacterStat = SavedData.PlayerSaveGameData.PlayerStat;
}

void AMainCharacterBase::SetWalkSpeedWithInterp(float NewValue, const float InterpSpeed, const bool bAutoReset)
{
	FTimerDelegate updateFunctionDelegate;
	
	//Binding the function with specific values
	updateFunctionDelegate.BindUFunction(this, FName("UpdateWalkSpeed"), NewValue, InterpSpeed, bAutoReset);
	
	//Calling MyUsefulFunction after 5 seconds without looping
	GetWorld()->GetTimerManager().ClearTimer(WalkSpeedInterpTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(WalkSpeedInterpTimerHandle, updateFunctionDelegate, 0.01f, true);
}

void AMainCharacterBase::UpdateWalkSpeed(const float TargetValue, const float InterpSpeed, const bool bAutoReset)
{
	float currentSpeed = GetCharacterMovement()->MaxWalkSpeed;
	if (FMath::IsNearlyEqual(currentSpeed, TargetValue, 0.1))
	{
		GetWorld()->GetTimerManager().ClearTimer(WalkSpeedInterpTimerHandle);
		if (bAutoReset)
		{
			SetWalkSpeedWithInterp(CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed) * 0.5f, InterpSpeed * 1.5f, false);
		}
	}
	currentSpeed = FMath::FInterpTo(currentSpeed, TargetValue, 0.1f, InterpSpeed);
	GetCharacterMovement()->MaxWalkSpeed = currentSpeed;
}

void AMainCharacterBase::SetFieldOfViewWithInterp(float NewValue, float InterpSpeed, const bool bAutoReset)
{
	FTimerDelegate updateFunctionDelegate;

	//Binding the function with specific values
	updateFunctionDelegate.BindUFunction(this, FName("UpdateFieldOfView"), NewValue, InterpSpeed, bAutoReset);

	//Calling MyUsefulFunction after 5 seconds without looping
	GetWorld()->GetTimerManager().ClearTimer(FOVInterpHandle);
	GetWorld()->GetTimerManager().SetTimer(FOVInterpHandle, updateFunctionDelegate, 0.01f, true);
}

void AMainCharacterBase::UpdateFieldOfView(const float TargetValue, float InterpSpeed, const bool bAutoReset)
{
	float currentFieldOfView = FollowingCamera->FieldOfView;
	if (FMath::IsNearlyEqual(currentFieldOfView, TargetValue, 0.1))
	{
		GetWorldTimerManager().ClearTimer(FOVInterpHandle);
		if (bAutoReset)
		{
			SetFieldOfViewWithInterp(90.0f, InterpSpeed * 1.5f, false);
		}
	}
	currentFieldOfView = FMath::FInterpTo(currentFieldOfView, TargetValue, 0.1f, 1.0f);
	FollowingCamera->SetFieldOfView(currentFieldOfView);
}

bool AMainCharacterBase::TryAddNewDebuff(FDebuffInfoStruct DebuffInfo, AActor* Causer)
{
	if (!Super::TryAddNewDebuff(DebuffInfo, Causer)) return false;

	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "Debuff");
	}

	FTimerDelegate timerDelegate; 

	timerDelegate.BindUFunction(this, "DeactivateBuffEffect");
	BuffEffectResetTimerHandle.Invalidate();
	GetWorld()->GetTimerManager().SetTimer(BuffEffectResetTimerHandle, timerDelegate, DebuffInfo.TotalTime, false);

	return true;
}

bool AMainCharacterBase::TryAddNewAbility(const int32 NewAbilityID)
{
	return AbilityManagerComponent->TryAddNewAbility(NewAbilityID);
}

bool AMainCharacterBase::TryRemoveAbility(const int32 NewAbilityID)
{
	return AbilityManagerComponent->TryRemoveAbility(NewAbilityID);
}

bool AMainCharacterBase::GetIsAbilityActive(const int32 AbilityID)
{
	return AbilityManagerComponent->GetIsAbilityActive(AbilityID);
}

bool AMainCharacterBase::EquipWeapon(const int32 NewWeaponID)
{
	return Super::EquipWeapon(NewWeaponID);
}

void AMainCharacterBase::SetFacialDamageEffect()
{
	/*UMaterialInstanceDynamic* currMaterial = CurrentDynamicMaterialList[1];
	
	if (currMaterial != nullptr && EmotionTextureList.Num() >= 3)
	{
		currMaterial->SetTextureParameterValue(FName("BaseTexture"), EmotionTextureList[(uint8)(EEmotionType::E_Angry)]);
		currMaterial->SetScalarParameterValue(FName("bIsDamaged"), true);
	}*/
}

void AMainCharacterBase::ResetFacialDamageEffect()
{
	/*UMaterialInstanceDynamic* currMaterial = CurrentDynamicMaterialList[1];

	if (currMaterial != nullptr && EmotionTextureList.Num() >= 3)
	{
		currMaterial->SetTextureParameterValue(FName("BaseTexture"), EmotionTextureList[(uint8)(EEmotionType::E_Idle)]);
		currMaterial->SetScalarParameterValue(FName("bIsDamaged"), false);
	}*/
}

void AMainCharacterBase::ClearAllTimerHandle()
{
	Super::ClearAllTimerHandle();

	GetWorldTimerManager().ClearTimer(BuffEffectResetTimerHandle);
	GetWorldTimerManager().ClearTimer(FacialEffectResetTimerHandle);
	GetWorldTimerManager().ClearTimer(RollDelayTimerHandle);
	GetWorldTimerManager().ClearTimer(DashDelayTimerHandle);
	GetWorldTimerManager().ClearTimer(JumpAttackDebuffTimerHandle);

	BuffEffectResetTimerHandle.Invalidate();
	FacialEffectResetTimerHandle.Invalidate();
	RollDelayTimerHandle.Invalidate();
	DashDelayTimerHandle.Invalidate();
	JumpAttackDebuffTimerHandle.Invalidate();
}