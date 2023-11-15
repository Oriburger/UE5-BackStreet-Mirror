// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/AIControllerBase.h"
#include "../../Item/public/WeaponBase.h"
#include "../../Global/public/BackStreetGameModeBase.h"
#include "../../Character/public/EnemyCharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"

AAIControllerBase::AAIControllerBase()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI_PERCEPTION"));

	InitAIPerceptionSystem();
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

void AAIControllerBase::InitAIPerceptionSystem()
{
	SightPerceptionConfig = CreateOptionalDefaultSubobject<UAISenseConfig_Sight>(TEXT("SIGHT_CONFIG"));
	SetPerceptionComponent(*AIPerceptionComponent);

	SightPerceptionConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightPerceptionConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightPerceptionConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightPerceptionConfig->SetMaxAge(MaxSightAge);
	SightPerceptionConfig->SightRadius = SightRadius;

	AIPerceptionComponent->SetDominantSense(*SightPerceptionConfig->GetSenseImplementation());
	GetPerceptionComponent()->ConfigureSense(*SightPerceptionConfig);
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AAIControllerBase::UpdateTargetPerception);
}

void AAIControllerBase::UpdateTargetPerception(AActor* Actor, FAIStimulus Stimulus)
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