// Fill out your copyright notice in the Description page of Project Settings.
#include "MainCharacterBase.h"
#include "../EnemyCharacter/EnemyCharacterBase.h"
#include "MainCharacterController.h"
#include "../Component/TargetingManagerComponent.h"
#include "../Component/ItemInventoryComponent.h"
#include "../Component/SkillManagerComponentBase.h"
#include "../Component/BoosterchipManagerComponent.h"
#include "../Component/PermanentUpgradeManager.h"
#include "../Component/ActionTrackingComponent.h"
#include "../Component/BattleApplicationManager.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/AbilitySystem/AbilityManagerBase.h"
#include "../../System/AssetSystem/AssetManagerBase.h"
#include "../../Item/ItemBase.h"
#include "../../Item/ItemBoxBase.h"
#include "../../Item/Weapon/Ranged/RangedCombatManager.h"
#include "../../Item/InteractiveCollisionComponent.h"
#include "../../System/MapSystem/Stage/GateBase.h"
#include "../../System/MapSystem/NewChapterManagerBase.h"
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

	SubCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SUB_CAMERA_BOOM"));
	SubCameraBoom->SetupAttachment(CameraBoom);
	SubCameraBoom->bUsePawnControlRotation = true;
	SubCameraBoom->TargetArmLength = 375.0f;
	SubCameraBoom->bInheritPitch = true;
	SubCameraBoom->bInheritRoll = false;
	SubCameraBoom->bInheritYaw = true;
	SubCameraBoom->bEnableCameraLag = true;
	SubCameraBoom->bEnableCameraRotationLag = true;
	SubCameraBoom->CameraRotationLagSpeed = 10.0f;
	SubCameraBoom->CameraLagSpeed = 10.0f;

	//MainCharacter RangedWeaponAim SpringArm
	RangedAimBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("RangedWeaponAim_Boom"));
	RangedAimBoom->SetupAttachment(RootComponent);				
	RangedAimBoom->TargetArmLength = 150.0f;					
	RangedAimBoom->bUsePawnControlRotation = true;
	RangedAimBoom->bInheritPitch = true;
	RangedAimBoom->bInheritRoll = false;
	RangedAimBoom->bInheritYaw = true;
	RangedAimBoom->SetRelativeLocation({ 0.0f, 80.0f, -10 });	//Set Location

	//MainCharacter RangedWeaponAim SpringArm
	TargetingModeBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("TargeingMode_Boom"));
	TargetingModeBoom->SetupAttachment(RootComponent);
	TargetingModeBoom->TargetArmLength = 150.0f;
	TargetingModeBoom->bUsePawnControlRotation = true;
	TargetingModeBoom->bInheritPitch = true;
	TargetingModeBoom->bInheritRoll = false;
	TargetingModeBoom->bInheritYaw = true;
	TargetingModeBoom->SetRelativeLocation({ 0.0f, 80.0f, -10 });	//Set Location

	HitSceneComponent->SetRelativeLocation(FVector(0.0f, 110.0f, 120.0f));

	//MainCharacter Main Camera
	FollowingCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FOLLOWING_CAMERA"));
	FollowingCamera->SetupAttachment(SubCameraBoom);
	FollowingCamera->bAutoActivate = true;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SOUND"));
	ItemInventory = CreateDefaultSubobject<UItemInventoryComponent>(TEXT("Item_Inventory"));

	TargetingManagerComponent = CreateDefaultSubobject<UTargetingManagerComponent>(TEXT("TARGETING_MANAGER"));;

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

int32 AMainCharacterBase::GetCurrentGearCount()
{
	if(IsValid(ItemInventory))
	{
		return ItemInventory->GetItemAmount(GEAR_ITEM_ID);
	}
	return 0;
}

// Called when the game starts or when spawned
void AMainCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	//Blueprint에서 추가된 컴포넌트 
	BoosterchipManagerComponent = FindComponentByClass<UBoosterchipManagerComponent>();
	PermanentUpgradeManager = FindComponentByClass<UPermanentUpgradeManager>();
	BattleApplicationManager = FindComponentByClass<UBattleApplicationManager>();

	InitCharacterGameplayInfo(DefaultStat);

	PlayerControllerRef = Cast<AMainCharacterController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));


	InitCombatUI();
	ItemInventory->InitInventory();
	
	//TargetingManagerComponent->OnTargetingActivated.AddDynamic(this, &AMainCharacterBase::OnTargetingStateUpdated);
	SetAutomaticRotateModeTimer();
}

// Called every frame
void AMainCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//0.1.0.2 임시 코드, 
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Die)
	{
		//UpdateVisibilityByDistance();
	}
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
		EnhancedInputComponent->BindAction(InputActionInfo.MoveAction, ETriggerEvent::Started, this, &AMainCharacterBase::OnMoveInputStarted);
		EnhancedInputComponent->BindAction(InputActionInfo.MoveAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Move);
		EnhancedInputComponent->BindAction(InputActionInfo.MoveAction, ETriggerEvent::Completed, this, &AMainCharacterBase::OnMoveInputEnded);

		//Look 
		EnhancedInputComponent->BindAction(InputActionInfo.LookAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Look);

		//Rolling
		EnhancedInputComponent->BindAction(InputActionInfo.RollAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Roll);

		//Attack
		EnhancedInputComponent->BindAction(InputActionInfo.AttackAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryAttack);

		//Sprint
		EnhancedInputComponent->BindAction(InputActionInfo.SprintAction, ETriggerEvent::Started, this, &AMainCharacterBase::Sprint);
		EnhancedInputComponent->BindAction(InputActionInfo.SprintAction, ETriggerEvent::Completed, this, &AMainCharacterBase::StopSprint);

		//Jump
		EnhancedInputComponent->BindAction(InputActionInfo.JumpAction, ETriggerEvent::Completed, this, &AMainCharacterBase::StartJump);

		//Zoom In&Out
		EnhancedInputComponent->BindAction(InputActionInfo.ZoomAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::ZoomIn);
		EnhancedInputComponent->BindAction(InputActionInfo.ZoomAction, ETriggerEvent::Completed, this, &AMainCharacterBase::ZoomOut);

		//Interaction
		EnhancedInputComponent->BindAction(InputActionInfo.InvestigateAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryInvestigate);
		
		//Lock On Mode
		EnhancedInputComponent->BindAction(InputActionInfo.LockOnAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::ToggleTargetingMode);
	}
}

void AMainCharacterBase::UpdateVisibilityByDistance()
{
	if (!ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_Skill)) return;
	//const float distance = (FollowingCamera->GetComponentLocation() - GetMesh()->GetComponentLocation()).Length();
	//SetActorHiddenInGame(distance <= 125.0f);
}

void AMainCharacterBase::SwitchWeapon(bool bSwitchToSubWeapon, bool bForceApply)
{
	if (!IsValid(ItemInventory)) return;
	if (!bForceApply && (!GetIsActionActive(ECharacterActionType::E_Idle) && !GetIsActionActive(ECharacterActionType::E_Shoot))) return;
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
		
		//play WeaponSwitch Sound
		if (AssetManagerBaseRef.IsValid())
		{
			AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "WeaponSwitch");
		}
	}
	else if (!bSwitchToSubWeapon && WeaponComponent->WeaponStat.WeaponType == EWeaponType::E_Shoot)
	{
		if (mainWeaponData.ItemID == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("SwitchWeapon %d Not Found"), mainWeaponData.ItemID);
			return;
		}
		WeaponComponent->InitWeapon(mainWeaponData.ItemID - 20000); //temp code

		//play WeaponSwitch Sound
		if (AssetManagerBaseRef.IsValid())
		{
			AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "WeaponSwitch");
		}
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
	UpdateMainCameraBoom(RangedAimBoom);

	OnZoomBegin.Broadcast(); //for ui (blueprint binding 가능)
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Aim);
}

void AMainCharacterBase::ZoomOut()
{
	if (WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Shoot) return;
	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Stun) SwitchWeapon(false, true);
	else SwitchWeapon(false);
	SetAimingMode(false);
	//FollowingCamera attach to CameraBoom Component using Interp ===================
	UpdateMainCameraBoom(CameraBoom);

	OnZoomEnd.Broadcast(); //for ui (blueprint binding 가능)
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Aim, true);
}

void AMainCharacterBase::TryShoot()
{
	if (WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Shoot) return;
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Shoot) return;

	if (AssetHardPtrInfo.ShootAnimMontageList[0] > 0
		&& IsValid(AssetHardPtrInfo.ShootAnimMontageList[0]))
	{
		PlayAnimMontage(AssetHardPtrInfo.ShootAnimMontageList[0], 1.0f);
		TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Shoot, false);
	}
}

void AMainCharacterBase::TryShootBySubCombo(FRotator ShootRotation, FVector ShootLocation)
{
	UE_LOG(LogTemp, Warning, TEXT("TryShootBySubCombo #1"));

	if (WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Shoot) return;

	UE_LOG(LogTemp, Warning, TEXT("TryShootBySubCombo #2"));

	WeaponComponent->RangedCombatManager->TryFireProjectile(ShootRotation, ShootLocation);
}

void AMainCharacterBase::SetAimingMode(bool bNewState)
{
	if (!GetIsActionActive(ECharacterActionType::E_Stun)) CharacterGameplayInfo.CharacterActionState = bNewState ? ECharacterActionType::E_Shoot : ECharacterActionType::E_Idle;

	GetCharacterMovement()->bOrientRotationToMovement = !bNewState;
	bUseControllerRotationYaw = bNewState;
	CharacterGameplayInfo.bIsAiming = bNewState;
	StopSprint(FInputActionValue(-1.0f)); //Aim 시, Sprint를 중지

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

void AMainCharacterBase::OnMoveInputStarted()
{
	if (OnMoveInputStateChanged.IsBound()) // 안전장치: 듣는 사람이 있을 때만
	{
		OnMoveInputStateChanged.Broadcast(true);
	}
}

void AMainCharacterBase::OnMoveInputEnded()
{
	if (OnMoveInputStateChanged.IsBound())
	{
		OnMoveInputStateChanged.Broadcast(false);
	}
	// 아래 기존 ResetMovementInputValue()코드
	MovementInputValue = FVector2D::ZeroVector;
	
	bMoveKeyDown = false;

	if (!bHoldToSprint)
	{
		FInputActionValue tempValue = FInputActionValue(-1.0f);
		StopSprint(tempValue);
	}
}

void AMainCharacterBase::Move(const FInputActionValue& Value)
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;

	// input is a Vector2D
	MovementInputValue = Value.Get<FVector2D>();
	if (MovementInputValue == FVector2D::ZeroVector) return;
	if (!ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_JumpAttack)) return;
	if (GetIsActionActive(ECharacterActionType::E_Stun)) return;

	if (Controller != nullptr)
	{
		FVector forwardAxis = FollowingCamera->GetRightVector().RotateAngleAxis(-90.0f, GetMesh()->GetUpVector());
		//!CharacterState.TargetedEnemy.IsValid() ? FollowingCamera->GetForwardVector()
		//					  : CharacterState.TargetedEnemy.Get()->GetActorLocation() - GetActorLocation();
		FVector rightAxis = UKismetMathLibrary::RotateAngleAxis(forwardAxis, 90.0f, GetActorUpVector());

		rightAxis.Z = forwardAxis.Z = 0.0f;

		AddMovementInput(forwardAxis, MovementInputValue.Y);
		AddMovementInput(FollowingCamera->GetRightVector(), MovementInputValue.X);

		TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Move);
		SetRotationLagSpeed(Value.Get<FVector2D>());

		//Update MoveKeyDown
		bMoveKeyDown = true;

		TWeakObjectPtr<UInteractiveCollisionComponent> oldInteractionCandidateRef = InteractionCandidateRef;
		InteractionCandidateRef = GetNearInteractionComponent();
		if (oldInteractionCandidateRef != InteractionCandidateRef && InteractionCandidateRef.IsValid())
		{
			if (oldInteractionCandidateRef.IsValid())
			{
				oldInteractionCandidateRef.Get()->SetInteractState(false);
			}
			InteractionCandidateRef.Get()->SetInteractState(true);
		}
		else if (!InteractionCandidateRef.IsValid() && oldInteractionCandidateRef.IsValid())
		{
			oldInteractionCandidateRef.Get()->SetInteractState(false);
		}
	}
}

void AMainCharacterBase::Look(const FInputActionValue& Value)
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr && !TargetingManagerComponent->GetIsTargetingActivated())
	{
		//Set invert Axis
		if (bInvertXAxis)
		{
			LookAxisVector.X = -LookAxisVector.X;
		}
		if (bInvertYAxis)
		{
			LookAxisVector.Y = -LookAxisVector.Y;
		}

		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X * (CameraSensivity * 0.5f + 1.0f));
		AddControllerPitchInput(LookAxisVector.Y * (CameraSensivity * 0.5f + 1.0f));
		SetManualRotateMode();
		if (!UGameplayStatics::IsGamePaused(GetWorld()))
			SetAutomaticRotateModeTimer();
	}
}

void AMainCharacterBase::StartJump(const FInputActionValue& Value)
{
	if (bIsJumplocked) return;
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (GetCharacterMovement()->IsFalling()) return;
	if (!ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_Skill)
		|| !ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_Roll)
		|| GetIsActionActive(ECharacterActionType::E_Stun)
		|| GetIsActionActive(ECharacterActionType::E_Die)
		|| GetIsActionActive(ECharacterActionType::E_KnockedDown)
		|| !ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_Aim)) return;
	
	ResetActionState(true);
	ActionTrackingComponent->ResetComboCount();
	GetMesh()->GetAnimInstance()->Montage_Stop(0.1f);

	Jump();
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Jump);
}

void AMainCharacterBase::Sprint(const FInputActionValue& Value)
{
	if (bIsSprintlocked) return;
	if (CharacterGameplayInfo.bIsAiming) return;
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (CharacterGameplayInfo.bIsSprinting)
	{
		if (!bHoldToSprint) StopSprint(FInputActionValue(-1.0f));
		return;
	}
	if (!ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_Sprint)) return;
	
	CharacterGameplayInfo.bIsSprinting = true;
	SetWalkSpeedWithInterp(CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed) * 1.25, 0.75f);

	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero()) return;

	ActionTrackingComponent->ResetComboCount();
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Sprint);
}

void AMainCharacterBase::StopSprint(const FInputActionValue& Value)
{
	if (bIsSprintlocked) return;
	if (CharacterGameplayInfo.bIsAiming) return;
	if (!CharacterGameplayInfo.bIsSprinting) return;
	CharacterGameplayInfo.bIsSprinting = false;
	SetWalkSpeedWithInterp(CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MoveSpeed), 0.5f);
	if (Value.GetValueType() == EInputActionValueType::Boolean && !bHoldToSprint) return;

	if (!ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_Sprint)) return;
	TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Sprint, true);
}

void AMainCharacterBase::Roll()
{	
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_Skill)) return; 
	if (!GetIsActionActive(ECharacterActionType::E_Idle) && !GetIsActionActive(ECharacterActionType::E_Attack)) return;
	if (GetIsActionActive(ECharacterActionType::E_Stun)
		|| GetIsActionActive(ECharacterActionType::E_Die)
		|| GetIsActionActive(ECharacterActionType::E_KnockedDown)) return;
	if (!CharacterGameplayInfo.bCanRoll) return;

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
	ResetLocationInterpTimer();


	// 시점 전환을 위해 제거
	//Rotation 리셋 로직
	GetWorldTimerManager().ClearTimer(RotationResetTimerHandle);
	SetActorRotation(newRotation + FRotator(0.0f, 90.0f, 0.0f));
	GetMesh()->SetWorldRotation(newRotation);

	//애니메이션 
	CharacterGameplayInfo.CharacterActionState = ECharacterActionType::E_Roll;
	if (AssetHardPtrInfo.RollAnimMontageList.Num() > 0
		&& IsValid(AssetHardPtrInfo.RollAnimMontageList[0]))
	{
		PlayAnimMontage(AssetHardPtrInfo.RollAnimMontageList[0], 1.0f);
		TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Roll);

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
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;

	if (InteractionCandidateRef.IsValid())
	{
		if (AssetHardPtrInfo.InvestigateAnimMontageList.Num() > 0
			&& IsValid(AssetHardPtrInfo.InvestigateAnimMontageList[0]))
		{
			PlayAnimMontage(AssetHardPtrInfo.InvestigateAnimMontageList[0]);
		}
		TryBroadcastActionTriggerDelegate(ECharacterActionType::E_Interact);
		InteractionCandidateRef.Get()->Interact();
		ResetActionState();
	}
}

void AMainCharacterBase::ToggleTargetingMode(const FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (!TargetingManagerComponent->GetIsTargetingActivated()
		&& !IsValid(TargetingManagerComponent->GetTargetedCandidate()))
	{
		if (Controller)
		{
			SetManualRotateMode();
			if (!UGameplayStatics::IsGamePaused(GetWorld()))
				SetAutomaticRotateModeTimer();

			float newYawValue = GetMesh()->GetComponentRotation().Yaw + 95.0f;
			Controller->SetControlRotation(FRotator(0.0f, newYawValue, 0.0f));
			TryBroadcastActionTriggerDelegate(ECharacterActionType::E_ResetCamera);
		}
		return;
	}

	bool result = false;
	USpringArmComponent* destinationBoom;
	if (TargetingManagerComponent->GetIsTargetingActivated())
	{
		destinationBoom = CameraBoom;
		result = true;
		TargetingManagerComponent->DeactivateTargeting();
		
		//Playsound from asssetmanager
		GamemodeRef.Get()->GetGlobalAssetManagerBaseRef()->PlaySingleSound(this, ESoundAssetType::E_System, 2000, "LockOnToggleOff");
		TryBroadcastActionTriggerDelegate(ECharacterActionType::E_LockOn, true);
	}
	else
	{
		destinationBoom = TargetingModeBoom;
		result = TargetingManagerComponent->ActivateTargeting();
		if (result)
		{
			GamemodeRef.Get()->GetGlobalAssetManagerBaseRef()->PlaySingleSound(this, ESoundAssetType::E_System, 2000, "LockOnToggleOn");
			TryBroadcastActionTriggerDelegate(ECharacterActionType::E_LockOn);
		}
	}

	//FollowingCamera attach to RangedAimBoom Component using Interp ==================
	if (result)
	{
		if (!CharacterGameplayInfo.bIsAiming)
			UpdateMainCameraBoom(destinationBoom);
	}
}

void AMainCharacterBase::SnapToCharacter(AActor* Target, float InterpSpeed)
{
	if (!IsValid(Target)) return;

	FVector startLocation = GetActorLocation();
	FVector endLocation = Target->GetActorLocation();
	endLocation.Z = Cast<AEnemyCharacterBase>(Target)->HitSceneComponent->GetComponentLocation().Z;
	FVector dirVector = endLocation - startLocation; dirVector.Normalize();
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(startLocation, endLocation));

	//pawn, worldstatic, worlddynamic
	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes = {
		EObjectTypeQuery::ObjectTypeQuery1,
		EObjectTypeQuery::ObjectTypeQuery3,
		EObjectTypeQuery::ObjectTypeQuery4 
	};
	SetLocationWithInterp(startLocation, startLocation + dirVector * (FVector::Distance(startLocation, endLocation) - 75.0f), objectTypes, 2.0f, false, true, true, false);
	if (GetDistanceTo(TargetingManagerComponent->GetTargetedCandidate()) > 400.0f)
	{
		SetFieldOfViewWithInterp(130.0f, 3.0f, true);
	}
}

float AMainCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (damageAmount > 0.0f && !ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_Attack) && !ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_JumpAttack)
		&& !ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_DashAttack))
	{
		PlayHitAnimMontage();
	}

	//Damage to AP Rate 적용
	float damageToAPRate = GetStatTotalValue(ECharacterStatType::E_DamageToAPRate);
	if(damageToAPRate > 0.0f)
	{
		float apGain = damageAmount * (damageToAPRate);
		float factor = FMath::Pow(10.0, 2);
		float actionPoint = ceil(apGain) * 0.01f;
		actionPoint = FMath::TruncToFloat(actionPoint * factor) / factor;
		
		CharacterGameplayInfo.CurrentAP += actionPoint;
		CharacterGameplayInfo.CurrentAP = FMath::Min(CharacterGameplayInfo.CurrentAP, CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MaxAP));
		OnAPAmountUpdated.Broadcast(actionPoint);
	}

	//TimerManager을 이용해 E_InvincibilityTime만큼 무적 처리 (Reset 로직 포함)
	//Material Param (OnDamageAmount) 변경 (Reset 포함)
	if (!GetCharacterGameplayInfo().bIsInvincibility)
	{
		CharacterGameplayInfo.bIsInvincibility = true;
		for (auto& material : AssetHardPtrInfo.DynamicMaterialInstanceList)
		{
			if(IsValid(material))
			{
				material->SetScalarParameterValue(FName("OnDamageAmount"), 1.0f);
			}
		}

		//reset using lambda
		GetWorldTimerManager().SetTimer(ResetInvisibilityHandle, FTimerDelegate::CreateLambda([=]()
		{
			CharacterGameplayInfo.bIsInvincibility = false;
			for (auto& material : AssetHardPtrInfo.DynamicMaterialInstanceList)
			{
				if (IsValid(material))
				{
					material->SetScalarParameterValue(FName("OnDamageAmount"), 0.0f);
				}
			}
		}), CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_InvincibilityTime), false);
	} 

	//Total Damage Taken 통계에 반영
	CharacterGameplayInfo.TotalDamageTaken += damageAmount;

	return damageAmount;
}

void AMainCharacterBase::TryAttack()
{
	if (UGameplayStatics::GetGlobalTimeDilation(GetWorld()) <= 0.01) return;
	if (CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Attack
		&& CharacterGameplayInfo.CharacterActionState != ECharacterActionType::E_Idle)
	{
		if (CharacterGameplayInfo.bIsAiming &&
			ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_Shoot))
		{
			TryShoot();
		}
		return;
	};
	if (!CharacterGameplayInfo.bCanAttack) return;
	if (WeaponComponent->WeaponID == 0)
	{
		GamemodeRef->PrintSystemMessage(ESystemMessageType::E_NoWeapon, true);
		return;
	}

	this->Tags.Add("Attack|Common");

	//===================== Attack Try 조건 체크 ==========================================
	if (!CharacterGameplayInfo.bCanAttack || !GetIsActionActive(ECharacterActionType::E_Idle)) return;
	int32 nextAnimIdx = ActionTrackingComponent->CurrentComboCount;
	int32 maxComboIdx = ActionTrackingComponent->CurrentComboType == EComboType::E_Normal ? GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_MaxNormalComboIdx)
						: ActionTrackingComponent->CurrentComboType == EComboType::E_Dash ? GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_MaxDashComboIdx)
						: ActionTrackingComponent->CurrentComboType == EComboType::E_Jump ? GetCharacterGameplayInfo().GetTotalValue(ECharacterStatType::E_MaxAerialComboIdx)
						: 0;
	maxComboIdx = maxComboIdx == 0 ? 1e9 : maxComboIdx; //0이면 무한으로 처리
	nextAnimIdx = nextAnimIdx % maxComboIdx;
	if (nextAnimIdx == 0 && (!ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_Attack)
		|| !ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_DashAttack)
		|| !ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_JumpAttack)))
		return; //

	//===================== Dash Combo 조건 체크 ==========================================
	if (CharacterGameplayInfo.bIsSprinting && !CharacterGameplayInfo.bIsAirAttacking
		&& !CharacterGameplayInfo.bIsDownwardAttacking && !GetCharacterMovement()->IsFalling()
		&& ActionTrackingComponent->GetIsActionReady(ECharacterActionType::E_DashAttack))
	{
		ActionTrackingComponent->CurrentComboType = EComboType::E_Dash;
	}

	//===================== Rotation 조정 ================================================
	if (MovementInputValue.Length() > 0)
	{
		float turnAngle = FMath::RadiansToDegrees(FMath::Atan2(MovementInputValue.X, MovementInputValue.Y))
			+ FollowingCamera->GetComponentRotation().Yaw;
		SetActorRotation(FRotator(0.0f, turnAngle, 0.0f));
	}

	// 2. 타겟 데이터 가져오기
	ACharacterBase* targetCharacter = Cast<ACharacterBase>(TargetingManagerComponent->GetTargetedCharacter());
	ACharacterBase* targetCandidate = Cast<ACharacterBase>(TargetingManagerComponent->GetTargetedCandidate());
	const bool bIsTargetingActivated = TargetingManagerComponent->GetIsTargetingActivated();
	const bool bIsActionLaunch = (ActionTrackingComponent->CurrentComboCount == 0); //콤보 카운트가 0일 때만 액션런치임

	// 3. 타겟팅된 적 처리 (우선순위 1순위)
	// 타겟팅이 켜져 있고, 돌진(Snap) 조건(거리, 상태 등)을 만족하는지 확인
	if (bIsTargetingActivated && GetCanPerformSnapCharacter(targetCharacter, bIsActionLaunch))
	{
		UE_LOG(LogTemp, Warning, TEXT("SnapToCharacter (Target): %s"), *targetCharacter->GetName());

		if (bIsActionLaunch)
		{
			TryBroadcastActionTriggerDelegate(ECharacterActionType::E_ActionLaunch);
		}
		SnapToCharacter(targetCharacter, bIsActionLaunch ? 3.0f : 1.0f);
	}

	// 4. 타겟 후보 처리 (우선순위 2순위)
	// 후보가 유효하고 '경계(Boundary)' 안에 있는지 우선 확인
	// (GetCanPerformSnapCharacter 내부에서도 체크하지만, 회전(Rotate) 로직을 위해 여기서도 필요)
	else if (IsValid(targetCandidate) && TargetingManagerComponent->GetIsEnemyInBoundary(targetCandidate->GetActorLocation()))
	{
		// 돌진(Snap) 조건을 완벽히 만족하면 -> 돌진
		if (GetCanPerformSnapCharacter(targetCandidate, bIsActionLaunch))
		{
			UE_LOG(LogTemp, Warning, TEXT("SnapToCharacter (Candidate): %s"), *targetCandidate->GetName());

			if (bIsActionLaunch)
			{
				TryBroadcastActionTriggerDelegate(ECharacterActionType::E_ActionLaunch);
			}
			SnapToCharacter(targetCandidate, bIsActionLaunch ? 3.0f : 1.0f);
		}
		// 돌진 조건은 안되지만 경계 안에는 있음 -> 회전만 수행
		else
		{
			FRotator lookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), targetCandidate->GetActorLocation());
			//SetActorRotation(FRotator(0.0f, lookAtRot.Yaw, 0.0f));
		}
	}

	Super::TryAttack();
}

bool AMainCharacterBase::TrySkill(int32 SkillID)
{
	//AP Check Test ==========================================
	//스킬 사용 시 2번 눌리는 문제 해결(꾹 누르면 연속적으로 발동 해결)
	if (ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_Skill)) return false;
	
	bool bIsInInventory = false;
	FSkillInfo skillInfo = SkillManagerComponent->GetSkillInfoData(SkillID, bIsInInventory);
	float skillAPCost = skillInfo.SkillAPCost;

	if (CharacterGameplayInfo.CharacterActionState == ECharacterActionType::E_Attack)
	{
		StopAttack();
	}
	if (CharacterGameplayInfo.CurrentAP - skillAPCost < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("current AP : %f"), CharacterGameplayInfo.CurrentAP);
		GamemodeRef->PrintSystemMessage(ESystemMessageType::E_NotEnoughAP, true);
		return false;
	}
	//====================================================
	bool result = Super::TrySkill(SkillID);

	if (result)
	{
		//타게팅 Candiate, Target 체크 후 회전 처리
		bool bIsTargetingActivated = TargetingManagerComponent->GetIsTargetingActivated();
		ACharacterBase* targetCharacter = Cast<ACharacterBase>(TargetingManagerComponent->GetTargetedCharacter());
		ACharacterBase* targetCandidate = Cast<ACharacterBase>(TargetingManagerComponent->GetTargetedCandidate());
		//타게팅이 활성화 되어있고, 타겟이 유효하다면
		if (bIsTargetingActivated && IsValid(targetCharacter))
		{
			FRotator lookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), targetCharacter->GetActorLocation());
			SetActorRotation(FRotator(0.0f, lookAtRot.Yaw, 0.0f));
		}
		//타겟은 없지만, 후보가 유효하다면
		else if (IsValid(targetCandidate))
		{
			FRotator lookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), targetCandidate->GetActorLocation());
			SetActorRotation(FRotator(0.0f, lookAtRot.Yaw, 0.0f));
		}
		//아무도 없다면
		else if (MovementInputValue.Length() > 0)
		{
			float turnAngle = FMath::RadiansToDegrees(FMath::Atan2(MovementInputValue.X, MovementInputValue.Y))
				+ FollowingCamera->GetComponentRotation().Yaw;
			SetActorRotation(FRotator(0.0f, turnAngle, 0.0f));
		}
		else
		{
			SetActorRotation(FRotator(0.0f, FollowingCamera->GetComponentRotation().Yaw, 0.0f));
		}

		//Skill AP Consumption
		CharacterGameplayInfo.CurrentAP -= skillAPCost;
		OnAPConsumed.Broadcast(skillAPCost);
	}

	return result;
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
}

void AMainCharacterBase::StandUp()
{
	Super::StandUp();
}

UInteractiveCollisionComponent* AMainCharacterBase::GetNearInteractionComponent()
{
	TArray<UInteractiveCollisionComponent*> returnList;
	TArray<UClass*> targetClassList = { UInteractiveCollisionComponent::StaticClass() };
	TEnumAsByte<EObjectTypeQuery> itemObjectType = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	FVector overlapBeginPos = GetActorLocation();
	
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
					return Cast<UInteractiveCollisionComponent>(component); //찾는 즉시 반환
			}
		}
	}
	return nullptr;
}

TArray<UAnimMontage*> AMainCharacterBase::GetTargetMeleeAnimMontageList()
{
	TArray<UAnimMontage*> targetAnimList;
	
	switch (ActionTrackingComponent->CurrentComboType)
	{
	case EComboType::E_Jump:
		targetAnimList = AssetHardPtrInfo.JumpComboAnimMontageList;
		break;
	case EComboType::E_Dash:
		targetAnimList = AssetHardPtrInfo.DashComboAnimMontageList;
		break;
	default:
		targetAnimList = Super::GetTargetMeleeAnimMontageList();
	}
	return targetAnimList;
}

bool AMainCharacterBase::GetCanPerformSnapCharacter(ACharacter* TargetActor, bool bIsActionLaunch)
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	// 이미 다른 액션 중인지 확인
	if (bIsActionLaunch && (
		ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_Attack) ||
		ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_DashAttack) ||
		ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_JumpAttack)))
	{
		return false;
	}

	// 타겟이 경계 안에 있는지 확인
	if (!TargetingManagerComponent->GetIsEnemyInBoundary(TargetActor->GetActorLocation()))
	{
		return false;
	}

	// 거리 체크
	const float distanceToTarget = GetDistanceTo(TargetActor);
	const float maxLaunchDistance = bIsActionLaunch ? GetStatTotalValue(ECharacterStatType::E_ActionLaunchDistance) : 400.0f;
	const float minLaunchDistance = 50.0f; // 매직 넘버는 const 변수나 매크로로 관리 추천

	return (distanceToTarget >= minLaunchDistance && distanceToTarget <= maxLaunchDistance);
}

void AMainCharacterBase::UpdateMainCameraBoom(USpringArmComponent* TargetBoom)
{
	if (!IsValid(TargetBoom)) return;
	//FollowingCamera attach to TargetBoom Component using Interp ==================
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	SubCameraBoom->AttachToComponent(TargetBoom, FAttachmentTransformRules::KeepWorldTransform);
	UKismetSystemLibrary::MoveComponentTo(SubCameraBoom, FVector(0, 0, 0), FRotator(0, 0, 0)
		, true, true, 0.2, false, EMoveComponentAction::Type::Move, LatentInfo);
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

void AMainCharacterBase::ResetCameraBoom()
{
	UpdateMainCameraBoom(CameraBoom);
}

void AMainCharacterBase::SetAutomaticRotateModeTimer()
{
	if (UGameplayStatics::IsGamePaused(GetWorld())) return;
	GetWorldTimerManager().ClearTimer(SwitchCameraRotateModeTimerHandle);
	SwitchCameraRotateModeTimerHandle.Invalidate();
	GetWorldTimerManager().SetTimer(SwitchCameraRotateModeTimerHandle, this, &AMainCharacterBase::SetAutomaticRotateMode, AutomaticModeSwitchTime);
}

void AMainCharacterBase::SetAutomaticRotateMode()
{
	if (TargetingManagerComponent->GetIsTargetingActivated() || UGameplayStatics::IsGamePaused(GetWorld())) return;
	
	float currentPitch = GetControlRotation().Pitch;
	if (currentPitch <= 0.0f || currentPitch >= 360.0f)
	{
		currentPitch = currentPitch + 720.0f;
		currentPitch = FMath::Fmod(currentPitch, 360.0f);
	}
	SetCameraVerticalAlignmentWithInterp(currentPitch >= 270.0f ? 360.0f : 0.0f, 0.14f);

	TWeakObjectPtr<AMainCharacterBase> weakThis(this);
	GetWorld()->GetTimerManager().SetTimer(RotationResetTimerHandle, FTimerDelegate::CreateLambda([weakThis]()
	{
		if (weakThis.IsValid())
		{
			weakThis.Get()->CameraBoom->bEnableCameraRotationLag = true;
			weakThis.Get()->CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
			weakThis.Get()->CameraBoom->bUsePawnControlRotation = false;
			weakThis.Get()->CameraBoom->bInheritYaw = true;
		}
	}), 1.5f, false /*반복*/);
}

void AMainCharacterBase::UpdateCameraPitch(float TargetPitch, float InterpSpeed)
{
	if (UGameplayStatics::IsGamePaused(GetWorld()) || CharacterGameplayInfo.bIsAiming)
	{
		SetManualRotateMode();
		return;
	}

	FRotator currentRotation = GetControlRotation();
	if (FMath::IsNearlyEqual(currentRotation.Pitch, TargetPitch, 5.0f) || GetWorld()->IsPaused())
	{
		GetWorld()->GetTimerManager().ClearTimer(CameraRotationAlignmentHandle);
	}
	currentRotation.Pitch = FMath::FInterpTo(currentRotation.Pitch, TargetPitch, 0.1f, InterpSpeed);
	if (currentRotation.Pitch <= 0.0f || currentRotation.Pitch >= 360.0f)
	{
		currentRotation.Pitch = currentRotation.Pitch + 720.0f;
		currentRotation.Pitch = FMath::Fmod(currentRotation.Pitch, 360.0f);
	}
	Controller->SetControlRotation(currentRotation);
}

void AMainCharacterBase::SetManualRotateMode()
{
	//clear automatic timer
	GetWorld()->GetTimerManager().ClearTimer(CameraRotationAlignmentHandle);
	GetWorld()->GetTimerManager().ClearTimer(RotationResetTimerHandle);
	CameraRotationAlignmentHandle.Invalidate();
	RotationResetTimerHandle.Invalidate();

	//Set manual properties
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
		CameraBoom->CameraRotationLagSpeed = NormalLagSpeed;
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

void AMainCharacterBase::UpdateDamageAmount(float NewDamage, float NewSubDamage)
{
	//기본값 처리
	NewDamage = NewDamage < 0.0f ? 0.0f : NewDamage;
	NewSubDamage = NewSubDamage < 0.0f ? 0.0f : NewSubDamage;

	CharacterGameplayInfo.TotalDamage += NewDamage;
	CharacterGameplayInfo.SubWeaponTotalDamage += NewSubDamage;

	if (ActionTrackingComponent->GetIsActionInProgress(ECharacterActionType::E_Skill)) return;
	//Calculate Action Gaze
	const float extraAPGainRate = CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_ExtraAPGainRate);
	float factor = FMath::Pow(10.0, 2);
	float actionPoint = ceil((NewDamage + NewSubDamage) * (1.0f + extraAPGainRate)) * 0.01f;
	actionPoint = FMath::TruncToFloat(actionPoint * factor) / factor;

	CharacterGameplayInfo.CurrentAP += actionPoint;
	CharacterGameplayInfo.CurrentAP = FMath::Min(CharacterGameplayInfo.CurrentAP, CharacterGameplayInfo.GetTotalValue(ECharacterStatType::E_MaxAP));

	OnDamageAmountUpdated.Broadcast(NewDamage + NewSubDamage);
	OnAPAmountUpdated.Broadcast(actionPoint);
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
	
	//InterpSpeed가 1.0f 이상일 경우, 타이머를 사용하지 않고 직접적으로 FieldOfView를 업데이트
	if (InterpSpeed >= 1.0f)
	{
		UpdateFieldOfView(NewValue, InterpSpeed, bAutoReset);
		return;
	}

	//Binding the function with specific values
	updateFunctionDelegate.BindUFunction(this, FName("UpdateFieldOfView"), NewValue, InterpSpeed, bAutoReset);

	//Calling MyUsefulFunction after 5 seconds without looping
	GetWorld()->GetTimerManager().ClearTimer(FOVInterpHandle);
	FOVInterpHandle.Invalidate();
	GetWorld()->GetTimerManager().SetTimer(FOVInterpHandle, updateFunctionDelegate, 0.01f, true);
}

void AMainCharacterBase::UpdateFieldOfView(const float TargetValue, float InterpSpeed, const bool bAutoReset)
{
	float currentFieldOfView = FollowingCamera->FieldOfView;
	if (FMath::IsNearlyEqual(currentFieldOfView, TargetValue, 0.1) && GetWorldTimerManager().IsTimerActive(FOVInterpHandle))
	{
		GetWorldTimerManager().ClearTimer(FOVInterpHandle);
		FOVInterpHandle.Invalidate();
		if (bAutoReset)
		{
			SetFieldOfViewWithInterp(90.0f, InterpSpeed * 1.5f, false);
		}
	}
	currentFieldOfView = FMath::FInterpTo(currentFieldOfView, TargetValue, 0.1f, InterpSpeed);
	FollowingCamera->SetFieldOfView(currentFieldOfView);
}

bool AMainCharacterBase::ApplyDebuffStack(FDebuffInfoStruct DebuffInfo, AActor* Causer)
{
	bool result = Super::ApplyDebuffStack(DebuffInfo, Causer);
	if(!result) return false;

	//Stun state management
	if (DebuffInfo.Type == ECharacterDebuffType::E_Stun)
	{
		FInputActionValue tempValue; 
		if (CharacterGameplayInfo.bIsAiming) ZoomOut();
		if (CharacterGameplayInfo.bIsSprinting) StopSprint(tempValue);
	}

	if (AssetManagerBaseRef.IsValid())
	{
		AssetManagerBaseRef.Get()->PlaySingleSound(this, ESoundAssetType::E_Character, 0, "Debuff");
	}

	FTimerDelegate timerDelegate; 

	timerDelegate.BindUFunction(this, "DeactivateBuffEffect");
	BuffEffectResetTimerHandle.Invalidate();
	GetWorld()->GetTimerManager().SetTimer(BuffEffectResetTimerHandle, timerDelegate, 1.0f, false);

	return true;
}

void AMainCharacterBase::ClearAllTimerHandle()
{
	Super::ClearAllTimerHandle();

	GetWorldTimerManager().ClearTimer(BuffEffectResetTimerHandle);
	GetWorldTimerManager().ClearTimer(FacialEffectResetTimerHandle);
	GetWorldTimerManager().ClearTimer(RollDelayTimerHandle);
	GetWorldTimerManager().ClearTimer(DashDelayTimerHandle);
	GetWorldTimerManager().ClearTimer(JumpAttackDebuffTimerHandle);
	GetWorldTimerManager().ClearTimer(WalkSpeedInterpTimerHandle);
	GetWorldTimerManager().ClearTimer(FOVInterpHandle);
	GetWorldTimerManager().ClearTimer(CameraRotationAlignmentHandle);
	GetWorldTimerManager().ClearTimer(RotationResetTimerHandle);
	GetWorldTimerManager().ClearTimer(ResetInvisibilityHandle);

	BuffEffectResetTimerHandle.Invalidate();
	FacialEffectResetTimerHandle.Invalidate();
	RollDelayTimerHandle.Invalidate();
	DashDelayTimerHandle.Invalidate();
	JumpAttackDebuffTimerHandle.Invalidate();
	WalkSpeedInterpTimerHandle.Invalidate();
	FOVInterpHandle.Invalidate();
	CameraRotationAlignmentHandle.Invalidate();
	RotationResetTimerHandle.Invalidate();
	ResetInvisibilityHandle.Invalidate();
}