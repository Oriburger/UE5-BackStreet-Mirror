// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/AIPathPatrolComponent.h"
#include "../public/PathPatrolPoint.h"

// Sets default values for this component's properties
UAIPathPatrolComponent::UAIPathPatrolComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

TPair<APathPatrolPoint*, int32> UAIPathPatrolComponent::GetNearestPointInfo()
{
	checkf(IsValid(GetOwner()), TEXT("Failed to get owner"));

	float minDistance = 9000000.0f;
	int32 idx = 0;
	APathPatrolPoint* result;
	
	//find neareat point in the list
	for (idx = 0; idx < PatrolPathInfoList.Num(); idx++)
	{
		if (IsValid(PatrolPathInfoList[idx].Point))
		{
			float targetDistance = (PatrolPathInfoList[idx].Point)->GetDistanceTo(GetOwner());
			if (targetDistance < minDistance)
			{
				minDistance = targetDistance;
				result = PatrolPathInfoList[idx].Point;
			}
		}
	}
	return { result, idx };
}

int32 UAIPathPatrolComponent::FindTargetPointIdx(APathPatrolPoint* Target)
{
	if (!IsValid(Target)) return -1;
	
	//Try to find 'target' in info list
	for (int32 idx = 0; idx < PatrolPathInfoList.Num(); idx++)
		if (PatrolPathInfoList[idx].Point == Target)
			return idx;
	//Failed to find 
	return -1;
}