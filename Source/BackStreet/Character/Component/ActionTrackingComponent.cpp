// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionTrackingComponent.h"
#include "../CharacterBase.h"

// Sets default values for this component's properties
UActionTrackingComponent::UActionTrackingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//====================================================================
//====== Basic Function ==============================================

// Called when the game starts
void UActionTrackingComponent::BeginPlay()
{
	Super::BeginPlay();

	//활성화 여부 판단
	if (GetOwner()->ActorHasTag("Player") || ForceEnableMonitorForEnemy)
		bIsEnabled = true;

	//캐릭터 레퍼런스 셋
	OwnerCharacterRef = Cast<ACharacterBase>(GetOwner()); 

	//타이머 이벤트 바인딩
	BindMonitorEvent();
}

void UActionTrackingComponent::BindMonitorEvent()
{
	if (!bIsEnabled) return;
	
	
}

void UActionTrackingComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	bIsEnabled = true; 
	BindMonitorEvent();
}

void UActionTrackingComponent::Deactivate()
{
	Super::Deactivate();
	bIsEnabled = false;
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}


// Called every frame
void UActionTrackingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


//======================================================================
//====== Monitor Function ==============================================

void UActionTrackingComponent::SetActionResetTimer(FTimerHandle& TimerHandle, float& TimeoutValue, bool& Target)
{
	FTimerDelegate timerDelegate;
	timerDelegate.BindUFunction(this, FName("ResetValue"), Target);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, timerDelegate, TimeoutValue, false);
}
