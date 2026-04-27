// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "GameFramework/Actor.h"
#include "PathPatrolPoint.generated.h"

UCLASS()
class BACKSTREET_API APathPatrolPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APathPatrolPoint();

protected:
	UPROPERTY()
		class UBillboardComponent* BillboardRoot;
};
