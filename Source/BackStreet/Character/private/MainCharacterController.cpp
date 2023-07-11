// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/MainCharacterController.h"
#include "../public/MainCharacterBase.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/KismetMathLibrary.h"

void AMainCharacterController::BeginPlay()
{
	Super::BeginPlay();

	this->bShowMouseCursor = true;
}

FRotator AMainCharacterController::GetAimingRotation()
{
	//임시 코드) 추후 Attach되자마자 Gamemode를 통해 UI, Controller 등으로 Delegate Broadcast
	//auto genericApplication = FSlateApplication::Get().GetPlatformApplication();
	//if (genericApplication.Get() != nullptr && genericApplication->IsGamepadAttached())
	//{
		//return GetRightAnalogRotation();
	//	return FRotator();
	//}
	return GetRotationToCursor();
}

FRotator AMainCharacterController::GetRotationToCursor()
{
	if (!IsValid(GetPawn())) return FRotator();

	FRotator retRotation = FRotator();
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
	FHitResult hitResult;

	//Trace의 시작은 마우스의 World Location
	DeprojectMousePositionToWorld(traceStartLocation, mouseDirection);

	//마우스 커서 위치에서 바닥으로의 INF만큼의 위치가 Trace의 마지막 지점
	traceEndLocation = traceStartLocation + mouseDirection * 250000.0f;

	//카메라에서 커서의 바닥 위치까지 LineTrace를 진행 -> 실제 커서의 월드 상호작용 위치가 hitResult.Location에 담김
	GetWorld()->LineTraceSingleByChannel(hitResult, traceStartLocation, traceEndLocation
		, ECollisionChannel::ECC_Camera);

	if (hitResult.bBlockingHit) return hitResult.Location;
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
