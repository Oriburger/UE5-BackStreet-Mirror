// Fill out your copyright notice in the Description page of Project Settings.


#include "AIControllerBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/EnemyCharacter/EnemyCharacterBase.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Prediction.h"
#include "Perception/AIPerceptionComponent.h"

AAIControllerBase::AAIControllerBase()
{
	
}

void AAIControllerBase::OnPossess(APawn* PossessedPawn)
{
	Super::OnPossess(PossessedPawn);

	if (IsValid(BehaviorTree))
	{
		RunBehaviorTree(BehaviorTree);
	}
}

void AAIControllerBase::BeginPlay()
{
	Super::BeginPlay();

	ABackStreetGameModeBase* gameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	gameModeRef->ClearResourceDelegate.AddDynamic(this, &AAIControllerBase::ClearAllTimerHandle);

	//���� ��, AI ������ ��� ���� �д�.
	if (IsValid(GetBrainComponent()))
	{
		DeactivateAI();
	}
}

void AAIControllerBase::ActivateAI()
{
	GetBrainComponent()->StartLogic();
}

void AAIControllerBase::DeactivateAI()
{
	GetBrainComponent()->StopLogic("Pause");
}

void AAIControllerBase::UpdateTargetPerception(AActor* Actor, FAIStimulus Stimulus)
{
	TSubclassOf<UAISense> senseClass = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);

	if (senseClass == UAISense_Sight::StaticClass())
		ProcessSight(Actor, Stimulus);

	else if(senseClass == UAISense_Hearing::StaticClass())
		ProcessHearing(Actor, Stimulus);

	else
		ProcessPrediction(Actor, Stimulus);
		
}

void AAIControllerBase::ProcessSight(AActor* Actor, FAIStimulus Stimulus)
{
	if (IsValid(Actor) && Actor->ActorHasTag("Player") && Stimulus.WasSuccessfullySensed())
	{
		GetWorldTimerManager().ClearTimer(SightLossTimerHandle);
		GetBlackboardComponent()->SetValueAsBool("HasLineOfSight", true);
		GetBlackboardComponent()->SetValueAsObject("TargetCharacter", Actor);
	}
	else
	{
		GetBlackboardComponent()->SetValueAsBool("HasLineOfSight", false);
		GetWorldTimerManager().SetTimer(SightLossTimerHandle, this, &AAIControllerBase::ResetTargetPerception, 1.0f, false, MaxSightAge);
	}
}

void AAIControllerBase::ResetTargetPerception()
{
	GetBlackboardComponent()->SetValueAsObject("TargetCharacter", nullptr);
}

void AAIControllerBase::ProcessHearing(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;
	GetBlackboardComponent()->SetValueAsBool("HasHeardSomething", true);
	GetBlackboardComponent()->SetValueAsVector("HearingLocation", Stimulus.StimulusLocation);
}

void AAIControllerBase::ProcessPrediction(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		GetBlackboardComponent()->SetValueAsBool("HasPrediction", true);
		GetBlackboardComponent()->SetValueAsVector("PredictLocation", Stimulus.StimulusLocation);
	}
	else
	{
		GetBlackboardComponent()->SetValueAsBool("HasPrediction", false);

		UObject* target = GetBlackboardComponent()->GetValueAsObject("TargetCharacter");
		FVector targetLocation = (target == nullptr ? FVector() : Cast<AActor>(target)->GetActorLocation());
		GetBlackboardComponent()->SetValueAsVector("PredictLocation", targetLocation);
	}
}

void AAIControllerBase::UpdateNewWeapon()
{
	if(!IsValid(GetPawn())) return;
	GetBlackboardComponent()->SetValueAsBool("HasRangedWeapon", Cast<ACharacterBase>(GetPawn())->WeaponComponent->GetWeaponStat().WeaponType != EWeaponType::E_Melee);
}

void AAIControllerBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(SightLossTimerHandle);
	SightLossTimerHandle.Invalidate();
}

bool AAIControllerBase::GetTurnDirection(bool Direction) 
{
	if (Direction) return TurnRight;
	else return TurnLeft;
}

void AAIControllerBase::SetTurnDirection(bool Right, bool Left) 
{
	TurnRight = Right;
	TurnLeft = Left;
}

EAIBehaviorType AAIControllerBase::GetBehaviorState()
{
	return AIBehaviorState;
}

void AAIControllerBase::SetBehaviorState(EAIBehaviorType ActionType)
{
	if (!IsValid(GetPawn())) return;
	GetBlackboardComponent()->SetValueAsEnum("AIBehaviorState", (uint8)ActionType);
	AIBehaviorState = ActionType;
	UE_LOG(LogTemp, Warning, TEXT("Set Behavior State"));
}
