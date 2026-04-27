// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterController.h"
#include "MainCharacterBase.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/KismetMathLibrary.h"

AMainCharacterController::AMainCharacterController()
{
}

void AMainCharacterController::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = false;
}

FRotator AMainCharacterController::GetAimingRotation()
{
	return FRotator();
}

FRotator AMainCharacterController::GetRotationToCursor()
{
	return FRotator();
}

FRotator AMainCharacterController::GetLastRotationToCursor()
{
	return FRotator();
}

FVector AMainCharacterController::GetCursorDeprojectionWorldLocation()
{
	return FVector();
}

bool AMainCharacterController::GetActionKeyIsDown(FName MappingName)
{
	return false;
}
