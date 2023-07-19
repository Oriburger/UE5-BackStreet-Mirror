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
#include "../../Character/public/CharacterInfoStruct.h"
#include "../../Character/public/EnemyStatInfoStruct.h"
#include "../../StageSystem/public/DirectionEnumInfo.h"
#include "../../StageSystem/public/StageInfoStruct.h"
#include "../../Item/public/ItemInfoStruct.h"
#include "../../Item/public/WeaponInfoStruct.h"
#include "../../Item/public/ProjectileInfoStruct.h"
#include "../../Global/public/AssetManagerBase.h"


#define MAX_CHAPTER_COUNT 2
#define MAX_ITEM_SPAWN 10
#define MIN_ITEM_SPAWN 7
#define MAX_STAGE_TYPE 5
#define MAX_GRID_SIZE 3