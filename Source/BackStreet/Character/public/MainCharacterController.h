// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "GameFramework/PlayerController.h"
#include "MainCharacterController.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API AMainCharacterController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
		virtual void BeginPlay() override;

public:
	//캐릭터가 조준하고 있는 Rotation을 반환
	UFUNCTION()
		FRotator GetAimingRotation();

	//UFUNCTION()
		//FRotator GetRightAnalogRotation();

	//캐릭터가 커서의 월드 방향을 바라보는 Rotation을 반환
	UFUNCTION()
		FRotator GetRotationToCursor();

	//마지막에 클릭이 된 커서로의 Rotation을 반환
	UFUNCTION()
		FRotator GetLastRotationToCursor();

	UFUNCTION()
		FVector GetCursorDeprojectionWorldLocation();

	//특정 Action 키가 아직 눌려있는지 확인
	UFUNCTION()
		bool GetActionKeyIsDown(FName MappingName);

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		bool bJoyStickMode = false;

private:
	UPROPERTY()
		FRotator LastRotationToCursor;
};
