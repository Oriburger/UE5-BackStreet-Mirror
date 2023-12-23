// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/MainCharacterBase.h"
#include "../public/MainCharacterController.h"
#include "../public/AbilityManagerBase.h"
#include "../../Global/public/DebuffManager.h"
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
#include "../../Item/public/RewardBoxBase.h"
#include "../../CraftingSystem/public/CraftBoxBase.h"
#include "Animation/AnimMontage.h"
#include "../../Global/public/SkillManagerBase.h"
#include "../public/CharacterBase.h"
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
	
	PlayerControllerRef = Cast<AMainCharacterController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	
	InitDynamicMeshMaterial(NormalMaterial);

	AbilityManagerRef = NewObject<UAbilityManagerBase>(this, UAbilityManagerBase::StaticClass(), FName("AbilityfManager"));
	AbilityManagerRef->InitAbilityManager(this);
}

// Called every frame
void AMainCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateWallThroughEffect();
}

// Called to bind functionality to input
void AMainCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Set up "movement" bindings.
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacterBase::MoveRight);
	PlayerInputComponent->BindAxis("ZoomIn", this, &AMainCharacterBase::ZoomIn);
	
	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &AMainCharacterBase::Roll);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMainCharacterBase::TryAttack);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &AMainCharacterBase::ReadyToThrow);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &AMainCharacterBase::Throw);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMainCharacterBase::TryReload);

	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &AMainCharacterBase::SwitchToNextWeapon);
	PlayerInputComponent->BindAction("PickItem", IE_Pressed, this, &AMainCharacterBase::TryInvestigate);
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &AMainCharacterBase::DropWeapon);

	
	PlayerInputComponent->BindAction("SelectSubWeaponA", IE_Pressed, this, &AMainCharacterBase::PickSubWeapon);
	PlayerInputComponent->BindAction("SelectSubWeaponB", IE_Pressed, this, &AMainCharacterBase::PickSubWeapon);
	PlayerInputComponent->BindAction("SelectSubWeaponC", IE_Pressed, this, &AMainCharacterBase::PickSubWeapon);
	PlayerInputComponent->BindAction("SelectSubWeaponD", IE_Pressed, this, &AMainCharacterBase::PickSubWeapon);
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
	for (FPredictProjectilePathPointData& point : predictProjectilePathResult.PathData)
	{
		DrawDebugSphere(GetWorld(), point.Location, 3.0f, 2, FColor::Red, false, 0.015f, 0, 3.0f);
	}
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

void AMainCharacterBase::MoveForward(float Value)
{
	FVector Direction = FVector(1.0f, 0.0f, 0.0f);
	AddMovementInput(Direction, Value);

	if (Value != 0)
	{
		if (OnMove.IsBound())
			OnMove.Broadcast();
	}
}

void AMainCharacterBase::MoveRight(float Value)
{
	FVector Direction = FVector(0.0f, 1.0f, 0.0f);
	AddMovementInput(Direction, Value);

	if (Value != 0)
	{
		if (OnMove.IsBound())
			OnMove.Broadcast();
	}
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
	if (RollSound->IsValidLowLevelFast())
	{
		UGameplayStatics::PlaySoundAtLocation(this, RollSound, GetActorLocation());
	}

	//애니메이션 
	CharacterState.CharacterActionState = ECharacterActionType::E_Roll;
	if (AnimAssetData.RollAnimMontageList.Num() > 0
		&& IsValid(AnimAssetData.RollAnimMontageList[0]))
	{
		PlayAnimMontage(AnimAssetData.RollAnimMontageList[0], FMath::Max(1.0f, CharacterStat.CharacterMoveSpeed / 500.0f));
	}

	if (OnRoll.IsBound())
		OnRoll.Broadcast();
}

void AMainCharacterBase::Dash()
{
	if (IsActorBeingDestroyed()) return;
	
	LaunchCharacter(FVector(0.0f, 0.0f, 500.0f), false, false);
	GetWorldTimerManager().SetTimer(DashDelayTimerHandle, FTimerDelegate::CreateLambda([&]() {
		const FVector& direction = GetMesh()->GetRightVector();
		float& speed = GetCharacterMovement()->MaxWalkSpeed;
		GetCharacterMovement()->Velocity = direction * (speed + 1000.0f);
	}), 0.075f, false);
}

void AMainCharacterBase::ZoomIn(float Value)
{
	if (Value == 0.0f) return;
	float newLength = CameraBoom->TargetArmLength;
	newLength = newLength + Value * 25.0f;
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
	}
	else if (TargetActor->ActorHasTag("ItemBox"))
	{
		Cast<AItemBoxBase>(TargetActor)->OnPlayerOpenBegin.Broadcast(this);
	}
	else if (TargetActor->ActorHasTag("RewardBox"))
	{
		Cast<ARewardBoxBase>(TargetActor)->OnPlayerBeginInteract.Broadcast(this);
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
	if (damageAmount > 0.0f && DamageCauser->ActorHasTag("Enemy"))
	{
		SetFacialDamageEffect(true);

		GetWorld()->GetTimerManager().ClearTimer(FacialEffectResetTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(FacialEffectResetTimerHandle, FTimerDelegate::CreateLambda([&]() {
			SetFacialDamageEffect(false);
		}), 1.0f, false);
	}
	return damageAmount;
}

void AMainCharacterBase::TryAttack()
{
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Attack
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;
	if (GetCurrentWeaponRef()->GetWeaponType() == EWeaponType::E_Throw) return;

	if (GetCurrentWeaponRef()->WeaponID == 0)
	{
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("무기가 없습니다.")), FColor::White);
		return;
	}

	//공격을 하고, 커서 위치로 Rotation을 조정
	this->Tags.Add("Attack|Common");
	Super::TryAttack();
	RotateToCursor();

	GetWorldTimerManager().ClearTimer(AttackLoopTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(AttackLoopTimerHandle, FTimerDelegate::CreateLambda([&]() {
		if (!IsActorBeingDestroyed() && PlayerControllerRef.Get()->GetActionKeyIsDown("Attack"))
			TryAttack(); //0.1초 뒤에 체크해서 계속 눌려있는 상태라면, Attack을 반복한다. 
	}), 0.1f, false);
}

void AMainCharacterBase::TrySkillAttack()
{
	check(GetCurrentWeaponRef() != nullptr);

	if (CharacterState.CharacterActionState != ECharacterActionType::E_Skill
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Idle) return;

	if (GetCurrentWeaponRef()->WeaponID == 0||GetCharacterState().CharacterCurrSkillGauge==0)
	{
		GamemodeRef->PrintSystemMessageDelegate.Broadcast(FName(TEXT("스킬을 사용할 수 없습니다. ")), FColor::White);
		return;
	}

	//공격을 하고, 커서 위치로 Rotation을 조정
	Super::TrySkillAttack();

	//Reset Weapon Skill Grade
	FWeaponStatStruct weaponStat = GetCurrentWeaponRef()->GetWeaponStat();
	weaponStat.SkillSetInfo.SkillGrade = ESkillGrade::E_None;
	GetCurrentWeaponRef()->SetWeaponStat(weaponStat);

	RotateToCursor();
}

void AMainCharacterBase::AddSkillGauge()
{
	check(GetCurrentWeaponRef() != nullptr);

	AWeaponBase* weaponRef = GetCurrentWeaponRef();
	CharacterState.CharacterCurrSkillGauge += weaponRef->GetWeaponStat().SkillGaugeInfo.SkillGaugeAug;
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
	if (CharacterState.CharacterActionState != ECharacterActionType::E_Idle
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Attack
		&& CharacterState.CharacterActionState != ECharacterActionType::E_Throw) return;

	FRotator newRotation = PlayerControllerRef.Get()->GetRotationToCursor();
	DrawDebugSphere(GetWorld(), PlayerControllerRef.Get()->GetCursorDeprojectionWorldLocation(), 10.0f, 5, FColor::Yellow, false, 1.0f, 1, 5);
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

void AMainCharacterBase::PickSubWeapon()
{
	int32 targetIdx = -1;
	if (PlayerControllerRef->GetActionKeyIsDown(FName("SelectSubWeaponA"))) targetIdx = 0;
	else if (PlayerControllerRef->GetActionKeyIsDown(FName("SelectSubWeaponB"))) targetIdx = 1;
	else if (PlayerControllerRef->GetActionKeyIsDown(FName("SelectSubWeaponC"))) targetIdx = 2;
	else if (PlayerControllerRef->GetActionKeyIsDown(FName("SelectSubWeaponD"))) targetIdx = 3;

	if (GetCurrentWeaponRef()->GetWeaponType() == EWeaponType::E_Throw && GetSubInventoryRef()->GetCurrentIdx() == targetIdx)
	{
		GetInventoryRef()->EquipWeaponByIdx(0);
	}
	else if (!TrySwitchToSubWeapon(targetIdx))
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
	Super::SwitchToNextWeapon();
}

void AMainCharacterBase::DropWeapon()
{
	Super::DropWeapon();
}

bool AMainCharacterBase::TryAddNewDebuff(ECharacterDebuffType NewDebuffType, AActor* Causer, float TotalTime, float Value)
{
	if (!Super::TryAddNewDebuff(NewDebuffType, Causer, TotalTime, Value)) return false;

	if (DebuffSound && BuffSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DebuffSound, GetActorLocation());
	}
	//230621 임시 제거
	//ActivateBuffNiagara(bIsDebuff, BuffDebuffType);

	GetWorld()->GetTimerManager().ClearTimer(BuffEffectResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(BuffEffectResetTimerHandle, FTimerDelegate::CreateLambda([&]() {
		DeactivateBuffEffect();
	}), TotalTime, false);

	return true;
}

bool AMainCharacterBase::TryAddNewAbility(const ECharacterAbilityType NewAbilityType)
{
	if(!IsValid(AbilityManagerRef)) return false;
	return AbilityManagerRef->TryAddNewAbility(NewAbilityType);
}

bool AMainCharacterBase::TryRemoveAbility(const ECharacterAbilityType TargetAbilityType)
{
	if (!IsValid(AbilityManagerRef)) return false;
	return AbilityManagerRef->TryRemoveAbility(TargetAbilityType);
}

bool AMainCharacterBase::GetIsAbilityActive(const ECharacterAbilityType TargetAbilityType)
{
	if (!IsValid(AbilityManagerRef)) return false;
	return AbilityManagerRef->GetIsAbilityActive(TargetAbilityType);
}

bool AMainCharacterBase::PickWeapon(int32 NewWeaponID)
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

void AMainCharacterBase::UpdateWallThroughEffect()
{
	if (!IsValid(GetWorld())) return;
	FHitResult hitResult;
	const FVector& traceBeginPos = FollowingCamera->GetComponentLocation();
	const FVector& traceEndPos = GetMesh()->GetComponentLocation();
	
	GetWorld()->LineTraceSingleByChannel(hitResult, traceBeginPos, traceEndPos, ECollisionChannel::ECC_Camera);

	if (hitResult.bBlockingHit && IsValid(hitResult.GetActor()))
	{
		if(!hitResult.GetActor()->ActorHasTag("Player") && !bIsWallThroughEffectActivated)
		{
			InitDynamicMeshMaterial(WallThroughMaterial);
			bIsWallThroughEffectActivated = true;
		}
		else if(hitResult.GetActor()->ActorHasTag("Player") && bIsWallThroughEffectActivated)
		{
			InitDynamicMeshMaterial(NormalMaterial);
			bIsWallThroughEffectActivated = false;
		}
	}
}

void AMainCharacterBase::SetFacialDamageEffect(bool NewState)
{
	UMaterialInstanceDynamic* currMaterial = CurrentDynamicMaterial;
	
	if (currMaterial != nullptr && EmotionTextureList.Num() >= 3)
	{
		currMaterial->SetTextureParameterValue(FName("BaseTexture"), EmotionTextureList[(uint8)(NewState ? EEmotionType::E_Angry : EEmotionType::E_Idle)]);
		currMaterial->SetScalarParameterValue(FName("bIsDamaged"), (float)NewState);
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
}
