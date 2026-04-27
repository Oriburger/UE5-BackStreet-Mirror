// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../../Global/BackStreet.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SetLocationWithInterp.generated.h"

UENUM(BlueprintType)
enum class ENotifyTraceType : uint8
{
	E_None						UMETA(DisplayName = "None"),
	E_Camera					UMETA(DisplayName = "Camera", ToolTip = "(플레이어 전용) Following Camera 기준으로 시작"),
	E_Mesh						UMETA(DisplayName = "Mesh", ToolTip = "캐릭터 위치를 기준으로 시작"),
	E_Character					UMETA(DisplayName = "Character", ToolTip="Character Root"),
};

UCLASS()
class BACKSTREET_API UAnimNotify_SetLocationWithInterp : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		bool bUseTraceCache = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Trace", meta = (EditCondition = "!bUseTraceCache"))
		float Distance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Trace", meta = (EditCondition = "!bUseTraceCache"))
		ENotifyTraceType TraceType = ENotifyTraceType::E_Character;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Trace", meta = (EditCondition = "!bUseTraceCache"))
		TArray< TEnumAsByte<EObjectTypeQuery> > ObjectTypeList;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Trace", meta = (EditCondition = "!bUseTraceCache"))
		bool bTraceComplex = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Interp")
		float InterpSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
		bool bDrawDebugInfo = false;

};
