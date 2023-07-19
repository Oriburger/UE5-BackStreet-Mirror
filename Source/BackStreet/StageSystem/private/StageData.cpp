// Fill out your copyright notice in the Description page of Project Settings.
#include "../public/StageData.h"

void AStageData::BeginPlay()
{
	Super::BeginPlay();
}

AStageData::AStageData()
{
	PrimaryActorTick.bCanEverTick = false;
}