// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SlashMultiEnemy.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UAnimNotifyState_SlashMultiEnemy : public UAnimNotifyState
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	/** Color of Notify in editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
		int32 SlashCount;

	/** Whether this notify state instance should fire in animation editors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = AnimNotify)
		float SlashSpeedRate;

#endif // WITH_EDITORONLY_DATA


};
