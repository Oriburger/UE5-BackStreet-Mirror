// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/* ----- 필수 라이브러리 ------------ */
#include "EngineMinimal.h"
#include "CollisionQueryParams.h"
#include "Perception/AIPerceptionTypes.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "TimerManager.h"

/* ----- Niagara 및 사운드 --------*/
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

/* ----- 구조체/Enum ------------ */
#include "../Character/Struct/CharacterInfoStruct.h"
#include "../Item/Struct/ItemInfoStruct.h"
#include "../Item/Struct/WeaponInfoStruct.h"
#include "../Item/Struct/ProjectileInfoStruct.h"
#include "../System/MapSystem/Struct/StageInfoStruct.h"
#include "../System/SkillSystem/Struct/SkillInfoStruct.h"
#include "../System/CraftingSystem/Struct/CraftingInfoStruct.h"
#include "../System/AssetSystem/Struct/AssetInfoStruct.h"
#include "../System/SaveSystem/Struct/SaveDataStruct.h"


/* ----- Plugin ------------ */
#include "CommonUserWidget.h"

#define MAX_CHAPTER_COUNT 1
#define MAX_ITEM_SPAWN 7
#define MIN_ITEM_SPAWN 5
#define MAX_STAGE_TYPE 5
#define MAX_GRID_SIZE 3


/* ----- Log --------------- */
DECLARE_LOG_CATEGORY_EXTERN(LogAbility, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogStage, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogItem, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSaveSystem, Log, All);

#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define BS_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
