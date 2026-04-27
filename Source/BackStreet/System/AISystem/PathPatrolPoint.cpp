// Fill out your copyright notice in the Description page of Project Settings.


#include "PathPatrolPoint.h"
#include "Components/BillboardComponent.h"

// Sets default values
APathPatrolPoint::APathPatrolPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BillboardRoot = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardRoot->SetRelativeLocation(FVector(0.0f));
	BillboardRoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BillboardRoot->bHiddenInGame = true;
	SetRootComponent(BillboardRoot);
}