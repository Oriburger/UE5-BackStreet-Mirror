// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/BTStateManageServiceBase.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "../../Item/public/WeaponBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIController.h"
#define INF 1e7f
#define MAX_CHASE_DISTANCE 1750.0f

UBTStateManageServiceBase::UBTStateManageServiceBase(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("UpdateAIState");
	RandomDeviation = 0.1f;
	Interval = 0.4f;

	bCreateNodeInstance = true;
	AIBehaviorState = EAIBehaviorType::E_Patrol;
}

void UBTStateManageServiceBase::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!OwnerCharacterRef.IsValid()) //최초 수행 : BB와 캐릭터 Ref를 초기화
	{
		OwnerCharacterRef = Cast<AEnemyCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
		OwnerCharacterRef.Get()->EnemyDamageDelegate.BindUFunction(this, FName("OnOwnerGetDamaged"));

		BlackboardRef = OwnerComp.GetBlackboardComponent();
		if (BlackboardRef.IsValid())
		{
			BlackboardRef.Get()->SetValueAsVector(FName("SpawnLocation"), OwnerCharacterRef.Get()->GetActorLocation());
		}
	}
	else 
	{
		UpdateAIState();

		if (BlackboardRef.IsValid())
		{
			BlackboardRef.Get()->SetValueAsEnum("AIBehaviorState", (uint8)AIBehaviorState);
		}
	}
}

void UBTStateManageServiceBase::UpdateAIState()
{
	if (!OwnerCharacterRef.IsValid()) return;

	// AI State를 잠시 멈추어야 할 경우 (E_Stun이 아니라 다른 이름이 더 좋을듯)
	if (OwnerCharacterRef.Get()->GetCharacterState().CharacterActionState == ECharacterActionType::E_Die
		|| OwnerCharacterRef.Get()->GetCharacterState().CharacterActionState == ECharacterActionType::E_Hit
		|| OwnerCharacterRef.Get()->GetCharacterState().CharacterActionState == ECharacterActionType::E_Stun)
	{
		AIBehaviorState = EAIBehaviorType::E_Stun;
		return;
	}

	if (CheckReturnState())
	{
		if (CheckPatrolState())
		{
			AIBehaviorState = EAIBehaviorType::E_Patrol;
		}
		else
		{
			AIBehaviorState = EAIBehaviorType::E_Return;
		}
	}
	else if (CheckChaseState())
	{
		AIBehaviorState = EAIBehaviorType::E_Chase;

		if (CheckAttackState())
		{
			AIBehaviorState = EAIBehaviorType::E_Attack;
		}
	}
	else
	{
		AIBehaviorState = EAIBehaviorType::E_Patrol;
	}
}

bool UBTStateManageServiceBase::CheckPatrolState()
{
	if (!BlackboardRef.IsValid()) return false;
	const FVector& spawnLocation = BlackboardRef.Get()->GetValueAsVector(FName("SpawnLocation"));
	if (GetDistanceTo(spawnLocation) <= 100.0f) return true;
	return false;
}

bool UBTStateManageServiceBase::CheckReturnState()
{
	if (!BlackboardRef.IsValid()) return false;
	const FVector& spawnLocation = BlackboardRef.Get()->GetValueAsVector(FName("SpawnLocation"));
	if (AIBehaviorState == EAIBehaviorType::E_Return || GetDistanceTo(spawnLocation) >= MAX_CHASE_DISTANCE) return true;
	return false;
}

bool UBTStateManageServiceBase::CheckChaseState()
{
	const ACharacter* targetCharacterRef = Cast<ACharacter>(BlackboardRef.Get()->GetValueAsObject(FName("TargetCharacter")));
	return IsValid(targetCharacterRef);
}

bool UBTStateManageServiceBase::CheckAttackState()
{
	if (!BlackboardRef.IsValid()) return false;
	if(!BlackboardRef.Get()->GetValueAsBool(FName("ReadyToAttack"))) return false;
	if(!BlackboardRef.Get()->GetValueAsBool(FName("PreChaseAnimFlag"))) return false;
	if( BlackboardRef.Get()->GetValueAsBool(FName("HasRangedWeapon")) && !BlackboardRef.Get()->GetValueAsBool(FName("HasLineOfSight"))) return false;

	ACharacter* targetCharacterRef = Cast<ACharacter>(BlackboardRef.Get()->GetValueAsObject(FName("TargetCharacter")));
	AWeaponBase* weaponActorRef = OwnerCharacterRef.Get()->GetWeaponActorRef();
	
	float distanceToTarget = IsValid(targetCharacterRef) ? GetDistanceTo(targetCharacterRef->GetActorLocation()) : 100.0f;
	float attackRange = IsValid(weaponActorRef) ? (weaponActorRef->GetAttackRange()) : 100.0f;
	return distanceToTarget <= attackRange;
}

float UBTStateManageServiceBase::GetDistanceTo(const FVector& EndLocation)
{
	if (!OwnerCharacterRef.IsValid()) return 50.0f;
	return UKismetMathLibrary::Vector_Distance(OwnerCharacterRef.Get()->GetActorLocation(), EndLocation);
}

void UBTStateManageServiceBase::OnOwnerGetDamaged(AActor* Causer)
{
	if (!BlackboardRef.IsValid()) return;
	if (!IsValid(Causer) || Causer->IsActorBeingDestroyed() || !Causer->ActorHasTag("Player")) return;
	if (AIBehaviorState == EAIBehaviorType::E_Stun) return;

	if (AIBehaviorState == EAIBehaviorType::E_Return)
	{
		BlackboardRef.Get()->SetValueAsVector("SpawnLocation", OwnerCharacterRef.Get()->GetActorLocation());
	}

	if (BlackboardRef.IsValid())	
	{
		AIBehaviorState = EAIBehaviorType::E_Chase;
		BlackboardRef.Get()->SetValueAsObject("TargetCharacter", Causer);
		BlackboardRef.Get()->SetValueAsEnum("AIBehaviorState", (uint8)AIBehaviorState);
	}
}
