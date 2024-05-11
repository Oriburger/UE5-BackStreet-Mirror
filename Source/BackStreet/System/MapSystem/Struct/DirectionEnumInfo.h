#pragma once

#include "Engine/UserDefinedEnum.h"


UENUM(BlueprintType)
enum class EDirection : uint8
{
	E_UP    UMETA(DisplayName = "UP"),
	E_DOWN  UMETA(DisplayName = "DOWN"),
	E_LEFT  UMETA(DisplayName = "LEFT"),
	E_RIGHT UMETA(DisplayName = "RIGHT"),
	E_Start UMETA(DisplayName = "Start"),
	E_Chapter UMETA(DisplayName = "Chapter")
};
