// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterController.h"
#include "MainCharacterBase.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/KismetMathLibrary.h"

AMainCharacterController::AMainCharacterController()
{
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AMainCharacterController::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = false;
}

FRotator AMainCharacterController::GetAimingRotation()
{
	//�ӽ� �ڵ�) ���� Attach���ڸ��� Gamemode�� ���� UI, Controller ������ Delegate Broadcast
	//auto genericApplication = FSlateApplication::Get().GetPlatformApplication();
	//if (genericApplication.Get() != nullptr && genericApplication->IsGamepadAttached())
	//{
		//return GetRightAnalogRotation();
	//	return FRotator::ZeroRotator;
	//}
	return GetRotationToCursor();
}

FRotator AMainCharacterController::GetRotationToCursor()
{
	if (!IsValid(GetPawn())) return FRotator::ZeroRotator;

	FRotator retRotation = FRotator::ZeroRotator;
	FVector cursorWorldDeprojectionLocation = GetCursorDeprojectionWorldLocation();

	retRotation = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), cursorWorldDeprojectionLocation);
	retRotation = UKismetMathLibrary::MakeRotator(0.0f, 0.0f, retRotation.Yaw + 270.0f);
	return LastRotationToCursor = retRotation;
}

FRotator AMainCharacterController::GetLastRotationToCursor()
{
	return LastRotationToCursor;
}

FVector AMainCharacterController::GetCursorDeprojectionWorldLocation()
{
	FVector traceStartLocation, traceEndLocation, mouseDirection;
	TArray<FHitResult> hitResultList;

	//Trace�� ������ ���콺�� World Location
	DeprojectMousePositionToWorld(traceStartLocation, mouseDirection);

	//���콺 Ŀ�� ��ġ���� �ٴ������� INF��ŭ�� ��ġ�� Trace�� ������ ����
	traceEndLocation = traceStartLocation + mouseDirection * 250000.0f;

	//ī�޶󿡼� Ŀ���� �ٴ� ��ġ���� LineTrace�� ���� -> ���� Ŀ���� ���� ��ȣ�ۿ� ��ġ�� hitResult.Location�� ���
	GetWorld()->LineTraceMultiByChannel(hitResultList, traceStartLocation, traceEndLocation
										, ECollisionChannel::ECC_Camera);// ECC_GameTraceChannel2);

	if (hitResultList.Num())
	{	
		for (FHitResult& hitResult : hitResultList)
		{
			if (!IsValid(hitResult.GetActor()) || hitResult.GetActor()->IsActorBeingDestroyed()) continue;
			if (hitResult.GetActor() == GetPawn()) continue;
			
			FString str = UKismetSystemLibrary::GetDisplayName(hitResult.GetActor());

			return hitResult.Location;
		}
	}
	return FVector();
}

bool AMainCharacterController::GetActionKeyIsDown(FName MappingName)
{
	TArray<FInputActionKeyMapping> actionKeyMappingList;
	UInputSettings::GetInputSettings()->GetActionMappingByName(MappingName, actionKeyMappingList);

	for (FInputActionKeyMapping& inputKey : actionKeyMappingList)
	{
		if (IsInputKeyDown(inputKey.Key))
		{
			return true;
		}
	}
	return false;
}