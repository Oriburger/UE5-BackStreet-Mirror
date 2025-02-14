// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/* ----- �ʼ� ���̺귯�� ------------ */
#include "EngineMinimal.h"
#include "CollisionQueryParams.h"
#include "Perception/AIPerceptionTypes.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "TimerManager.h"

/* ----- Niagara �� ���� --------*/
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

/* ----- ����ü/Enum ------------ */
#include "../Character/Struct/CharacterInfoStruct.h"
#include "../Item/Struct/ItemInfoStruct.h"
#include "../Item/Struct/WeaponInfoStruct.h"
#include "../Item/Struct/ProjectileInfoStruct.h"
#include "../System/MapSystem/Struct/StageInfoStruct.h"
#include "../System/SkillSystem/Struct/SkillInfoStruct.h"
#include "../System/CraftingSystem/Struct/CraftingInfoStruct.h"
#include "../System/AssetSystem/Struct/AssetInfoStruct.h"


/* ----- Plugin ------------ */
#include "CommonUserWidget.h"

#define MAX_CHAPTER_COUNT 1
#define MAX_ITEM_SPAWN 7
#define MIN_ITEM_SPAWN 5
#define MAX_STAGE_TYPE 5
#define MAX_GRID_SIZE 3

#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define BS_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] %s"), LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
DECLARE_LOG_CATEGORY_EXTERN(LogBS, Log, All)