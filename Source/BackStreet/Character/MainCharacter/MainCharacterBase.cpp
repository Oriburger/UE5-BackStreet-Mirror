// Fill out your copyright notice in the Description page of Project Settings.
#include "MainCharacterBase.h"
#include "MainCharacterController.h"
#include "../Component/TargetingManagerComponent.h"
#include "../Component/ItemInventoryComponent.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../System/SaveSystem/BackStreetGameInstance.h"
#include "../../System/AbilitySystem/AbilityManagerBase.h"
#include "../../System/CraftingSystem/CraftBoxBase.h"
#include "../../System/CraftingSystem/CraftingManagerComponent.h"
#include "../../System/AssetSystem/AssetManagerBase.h"
#include "../../Item/ItemBase.h"
#include "../../Item/ItemBoxBase.h"
#include "../../Item/RewardBoxBase.h"
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
	FollowingCamera->bAutoActivate = true;

	BuffNiagaraEmitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BUFF_EFFECT"));
	BuffNiagaraEmitter->SetupAttachment(GetMesh());
	BuffNiagaraEmitter->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f));
	BuffNiagaraEmitter->bAutoActivate = false;

	DirectionNiagaraEmitter = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DIRECTION_EFFECT"));
	DirectionNiagaraEmitter->SetupAttachment(GetMesh());
	DirectionNiagaraEmitter->SetRelativeRotation({ 0.0f, 90.0f, 0.0f });

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SOUND"));

	ItemInventory = CreateDefaultSubobject<UItemInventoryComponent>(TEXT("Item_Inventory"));

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
	InitCharacterState();
	ItemInventory->InitInventory();

	TargetingManagerComponent->OnTargetingActivated.AddDynamic(this, &AMainCharacterBase::OnTargetingStateUpdated);
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

		//Reload
		EnhancedInputComponent->BindAction(InputActionInfo.ReloadAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryReload);

		//Sprint
		EnhancedInputComponent->BindAction(InputActionInfo.SprintAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Sprint);
		EnhancedInputComponent->BindAction(InputActionInfo.SprintAction, ETriggerEvent::Completed, this, &AMainCharacterBase::StopSprint);

		//Jump
		EnhancedInputComponent->BindAction(InputActionInfo.JumpAction, ETriggerEvent::Completed, this, &AMainCharacterBase::StartJump);

		//Zoom
		//EnhancedInputComponent->BindAction(InputActionInfo.ZoomAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::ZoomIn);

		//Throw
		EnhancedInputComponent->BindAction(InputActionInfo.ThrowReadyAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::ReadyToThrow);
		EnhancedInputComponent->BindAction(InputActionInfo.ThrowAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Throw);

		//Interaction
		EnhancedInputComponent->BindAction(InputActionInfo.InvestigateAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::TryInvestigate);

		//Inventory
		//EnhancedInputComponent->BindAction(InputActionInfo.SwitchWeaponAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::SwitchToNextWeapon);
		//EnhancedInputComponent->BindAction(InputActionInfo.DropWeaponAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::DropWeapon);

		//SubWeapon
		//EnhancedInputComponent->BindAction(InputActionInfo.PickSubWeaponAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::PickSubWeapon);

		EnhancedInputComponent->BindAction(LockToTargetAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::LockToTarget);
	}
}

void AMainCharacterBase::ReadyToThrow()
{
}

void AMainCharacterBase::Throw()
{
}

void AMainCharacterBase::SetAimingMode(bool bNewState)
{
	
}

void AMainCharacterBase::UpdateAimingState()
{	
}

FVector AMainCharacterBase::GetThrowDestination()
{
	return FVector();
}

void AMainCharacterBase::ResetMovementInputValue()
{
	MovementInputValue = FVector2D::ZeroVector;
}

void AMainCharacterBase::Move(const FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (CharacterState.bIsAirAttacking || CharacterState.bIsDownwardAttacking) return;

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

		if (MovementInputValue.Length() > 0 && OnMove.IsBound())
		{
			OnMove.Broadcast();
		}
		SetRotationLagSpeed(Value.Get<FVector2D>());
	}
}

void AMainCharacterBase::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr && !TargetingManagerComponent->GetIsTargetingActivated())
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X / 2.0f);
		AddControllerPitchInput(LookAxisVector.Y / 2.0f);
		SetManualRotateMode();

		// set timer for automatic rotate mode 
		if (LookAxisVector.Length() <= 0.5f)
		{
			GetWorldTimerManager().ClearTimer(SwitchCameraRotateModeTimerHandle);
			SwitchCameraRotateModeTimerHandle.Invalidate();
			GetWorldTimerManager().SetTimer(SwitchCameraRotateModeTimerHandle, this, &AMainCharacterBase::SetAutomaticRotateMode, AutomaticModeSwitchTime);
		}
	}
}

void AMainCharacterBase::StartJump(const FInputActionValue& Value)
{
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	//CharacterState.CharacterActionState = ECharacterActionType::E_Jump;
	Jump();
}

void AMainCharacterBase::Sprint(const FInputActionValue& Value)
{
	if (CharacterState.bIsSprinting) return;
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero()) return;
	
	WeaponComponent->ResetComboCnt();
	CharacterState.bIsSprinting = true;
	SetWalkSpeedWithInterp(CharacterStat.DefaultMoveSpeed, 0.75f);
	SetFieldOfViewWithInterp(105.0f, 0.25f);
}

void AMainCharacterBase::StopSprint(const FInputActionValue& Value)
{
	if (!CharacterState.bIsSprinting) return;

	CharacterState.bIsSprinting = false;
	SetWalkSpeedWithInterp(CharacterStat.DefaultMoveSpeed * 0.75f, 0.4f);
	SetFieldOfViewWithInterp(90.0f, 0.5f);
}

void AMainCharacterBase::Roll()
{	
	if (!GetIsActionActive(ECharacterActionType::E_Idle) && !GetIsActionActive(ECharacterActionType::E_Attack)) return;
	if (CharacterState.bIsAirAttacking || CharacterState.bIsDownwardAttacking) return;

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
	CharacterState.CharacterActionState = ECharacterActionType::E_Roll;
	if (AssetHardPtrInfo.RollAnimMontageList.Num() > 0
		&& IsValid(AssetHardPtrInfo.RollAnimMontageList[0]))
	{
		PlayAnimMontage(AssetHardPtrInfo.RollAnimMontageList[0], 1.0f);
	}

	if (OnRoll.IsBound())
	{
		OnRoll.Broadcast();
	}		
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
		if (AssetHardPtrInfo.InvestigateAnimMontageList.Num() > 0
			&& IsValid(AssetHardPtrInfo.InvestigateAnimMontageList[0]))
		{
			PlayAnimMontage(AssetHardPtrInfo.InvestigateAnimMontageList[0]);
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

		//Cast<ACraftBoxBase>(TargetActor)->OnPlayerOpenBegin.Broadcast(this);
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

void AMainCharacterBase::LockToTarget(const FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (CharacterState.bIsAirAttacking || CharacterState.bIsDownwardAttacking) return;

	TargetingManagerComponent->ActivateTargeting();
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
	OnTakeDamage.Broadcast();
	return damageAmount;
}

void AMainCharacterBase::TryAttack()
{
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Attack
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (WeaponComponent->WeaponStat.WeaponType == EWeaponType::E_Throw) return;

	//IndieGo용 임시 코드----------------------------------------------------------
	if (WeaponComponent->WeaponStat.WeaponID == 12130) return;
	//---------------------------------------------------------------------------------

	if (WeaponComponent->WeaponID == 0)
	{
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("무기가 없습니다.")), FColor::White);
		return;
	}
	this->Tags.Add("Attack|Common");

	if (CharacterState.bIsSprinting && !CharacterState.bIsAirAttacking
		&& !CharacterState.bIsDownwardAttacking && !GetCharacterMovement()->IsFalling())
	{
		TryDashAttack();
		return;
	}

	//Rotate to attack direction using input (1. movement / 2. camera)
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Idle)
	{
		if (MovementInputValue.Length() > 0)
		{
			float turnAngle = FMath::RadiansToDegrees(FMath::Atan2(MovementInputValue.X, MovementInputValue.Y))
				+ FollowingCamera->GetComponentRotation().Yaw;
			SetActorRotation(FRotator(0.0f, turnAngle, 0.0f));
		}
		else
		{
			SetActorRotation(FRotator(0.0f, FollowingCamera->GetComponentRotation().Yaw, 0.0f));
		}
	}

	Super::TryAttack();
}

void AMainCharacterBase::TryUpperAttack()
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (CharacterState.bIsAirAttacking) return;
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (CharacterState.bIsSprinting)
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
	if (!GetCharacterMovement()->IsFalling() || !CharacterState.bIsAirAttacking) return;
	if (IsValid(TargetingManagerComponent->GetTargetedCharacter()))
	{
		SetFieldOfViewWithInterp(110.0f, 0.75f);
	}
}

bool AMainCharacterBase::TrySkill(int32 SkillID)
{	
	if (GetIsActionActive(ECharacterActionType::E_Attack))
	{
		ResetActionState();
	}
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Skill
		|| CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return false;

	return Super::TrySkill(SkillID);
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

void AMainCharacterBase::RotateToCursor()
{
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Attack) return;
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Throw) return;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	FRotator newRotation = GetControlRotation();
	newRotation.Pitch = newRotation.Roll = 0.0f;
	SetActorRotation(newRotation);
	return;
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

void AMainCharacterBase::SetCameraVerticalAlignmentWithInterp(float TargetPitch, const float InterpSpeed)
{
	FTimerDelegate updateFunctionDelegate;

	//Binding the function with specific values
	updateFunctionDelegate.BindUFunction(this, FName("UpdateCameraPitch"), TargetPitch, InterpSpeed);

	//Calling MyUsefulFunction after 5 seconds without looping
	GetWorld()->GetTimerManager().ClearTimer(CameraRotationAlignmentHandle);
	GetWorld()->GetTimerManager().SetTimer(CameraRotationAlignmentHandle, updateFunctionDelegate, 0.01f, true);
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
	CharacterStat = SavedData.PlayerSaveGameData.PlayerStat;
}

void AMainCharacterBase::ResetRotationToMovement()
{
	/* 시점 전환을 위해 제거
	if (CharacterState.CharacterActionState == ECharacterActionType::E_Roll) return;
	FRotator newRotation = GetCapsuleComponent()->GetComponentRotation();
	newRotation.Yaw += 270.0f;
	GetMesh()->SetWorldRotation(newRotation);
	GetCharacterMovement()->bOrientRotationToMovement = true;*/
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
			SetWalkSpeedWithInterp(CharacterStat.DefaultMoveSpeed * 0.5f, InterpSpeed * 1.5f, false);
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
	
	ActivateDebuffNiagara((uint8)DebuffInfo.Type);

	GetWorld()->GetTimerManager().ClearTimer(BuffEffectResetTimerHandle);
	BuffEffectResetTimerHandle.Invalidate();
	GetWorld()->GetTimerManager().SetTimer(BuffEffectResetTimerHandle, FTimerDelegate::CreateLambda([&]() {
		DeactivateBuffEffect();
	}), DebuffInfo.TotalTime, false);

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
	return Super::PickWeapon(NewWeaponID);
}

void AMainCharacterBase::ActivateDebuffNiagara(uint8 DebuffType)
{
	TArray<UNiagaraSystem*>& targetEmitterList = AssetHardPtrInfo.DebuffNiagaraEffectList;

	if (targetEmitterList.IsValidIndex(DebuffType) && targetEmitterList[DebuffType] != nullptr)
	{
		BuffNiagaraEmitter->SetRelativeLocation(FVector(0.0f, 0.0f, 125.0f));
		BuffNiagaraEmitter->Deactivate();
		BuffNiagaraEmitter->SetAsset((targetEmitterList)[DebuffType], false);
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
	GetWorldTimerManager().ClearTimer(RollTimerHandle); 
	GetWorldTimerManager().ClearTimer(DashDelayTimerHandle);

	BuffEffectResetTimerHandle.Invalidate();
	FacialEffectResetTimerHandle.Invalidate();
	RollTimerHandle.Invalidate();
	DashDelayTimerHandle.Invalidate();
}