#pragma once

#include "Engine/DataTable.h"
#include "StageInfoStruct.generated.h"

UENUM(BlueprintType)
enum class EStageCategoryInfo : uint8
{
	E_None				  	UMETA(DisplayName = "None"),
	E_Lobby					UMETA(DisplayName = "Lobby"),

	E_Entry					UMETA(DisplayName = "Entry"),
	E_Combat				UMETA(DisplayName = "Combat"),
	E_TimeAttack			UMETA(DisplayName = "TimeAttack"),
	E_EliteCombat			UMETA(DisplayName = "EliteCombat"),
	E_EliteTimeAttack		UMETA(DisplayName = "EliteTimeAttack"),
	E_Boss					UMETA(DisplayName = "Boss"),

	E_Craft					UMETA(DisplayName = "Craft"),
	E_MiniGame				UMETA(DisplayName = "MiniGame"),
	E_Gatcha				UMETA(DisplayName = "Gatcha"),
};

UENUM(BlueprintType)
enum class EWaveCategoryInfo : uint8
{
	E_None				  		UMETA(DisplayName = "None"),
	E_NomalWave					UMETA(DisplayName = "E_NomalWave"),
	E_TimeLimitWave			  	UMETA(DisplayName = "E_TimeLimitWave"),
};

UENUM(BlueprintType)
enum class EChapterLevel : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Chapter1			UMETA(DisplayName = "Chapter1"),
	E_Chapter2			UMETA(DisplayName = "Chapter2"),
	E_Chapter3			UMETA(DisplayName = "Chapter3"),
	E_Chapter4			UMETA(DisplayName = "Chapter4")
};
