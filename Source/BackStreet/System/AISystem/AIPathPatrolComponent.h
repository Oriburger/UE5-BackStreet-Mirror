// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIPathPatrolComponent.generated.h"

USTRUCT(BlueprintType)
struct FPathPatrolInfoStruct
{
public:
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly)
		class APathPatrolPoint* Point;

	UPROPERTY(EditInstanceOnly)
		float DelayValue;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UAIPathPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAIPathPatrolComponent();

	//Get nearest point in patrol path map from owner pawn
	//Nearest Point, its Index of array 'PatrolPathInfoList'
	TPair<class APathPatrolPoint*, int32>  GetNearestPointInfo();

	UFUNCTION()
		int32 FindTargetPointIdx(class APathPatrolPoint* Target);

public:
	//You have to initialize this Container with placing instances of APathPatrolPoint to world.
	//It doesn't support change patrol point dynamically in runtime.
	//Key value is delay value before going to next point.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		TArray<FPathPatrolInfoStruct> PatrolPathInfoList;

	//
	UPROPERTY(BlueprintReadWrite)
		int32 CurrentPointIdx;

	//UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
		//TMap<class APathPatrolPoint*, float> PatrolPathMap;
};