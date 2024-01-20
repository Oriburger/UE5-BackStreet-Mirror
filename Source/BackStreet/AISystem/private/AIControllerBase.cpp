// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/AIControllerBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Prediction.h"
#include "Perception/AIPerceptionComponent.h"

AAIControllerBase::AAIControllerBase()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI_PERCEPTION"));
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

	//시작 시, AI 로직을 잠시 멈춰 둔다.
	if (IsValid(GetBrainComponent()))
	{
		//GetBrainComponent()->PauseLogic(FString("PrevGameStart"));
		DeactivateAI();
	}
}

void AAIControllerBase::ActivateAI()
{
	//GetBrainComponent()->ResumeLogic(FString("GameStart")); //Delegate를 통해 Chapter이 초기화 되면, Activate 한다.
	GetBrainComponent()->StartLogic();
	//UE_LOG(LogTemp, Warning, TEXT("ActivateAI"));
}

void AAIControllerBase::DeactivateAI()
{
	GetBrainComponent()->StopLogic(FString("GameOver"));
}

void AAIControllerBase::UpdateTargetPerception(AActor* Actor, FAIStimulus Stimulus)
{
	TSubclassOf<UAISense> senseClass = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);

	if (senseClass == UAISense_Sight::StaticClass())
		ProcessSight(Actor, Stimulus);

	else if(senseClass == UAISense_Hearing::StaticClass())
		ProcessHearing(Actor, Stimulus);

	else
	{
		ProcessPrediction(Actor, Stimulus);
		UE_LOG(LogTemp, Warning, TEXT("Prediction!!!!"));
	}
		
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
		GetWorldTimerManager().SetTimer(SightLossTimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				GetBlackboardComponent()->SetValueAsObject("TargetCharacter", nullptr);
			}), 1.0f, false, MaxSightAge);
	}
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

	AWeaponBase* weaponActorRef = Cast<ACharacterBase>(GetPawn())->GetCurrentWeaponRef();

	if (IsValid(weaponActorRef))
	{
		GetBlackboardComponent()->SetValueAsBool("HasRangedWeapon", weaponActorRef->GetWeaponStat().WeaponType != EWeaponType::E_Melee);
	}		
}

void AAIControllerBase::ClearAllTimerHandle()
{
	GetWorldTimerManager().ClearTimer(SightLossTimerHandle);
}