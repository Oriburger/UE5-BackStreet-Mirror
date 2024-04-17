// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/public/BackStreet.h"
#include "CharacterBase.h"
#include "EnemyCharacterBase.h"
#include "BossCharacterBase.generated.h"

UCLASS()
class BACKSTREET_API ABossCharacterBase : public AEnemyCharacterBase
{
	GENERATED_BODY()

// ------ Global, Component ------------
public:
	ABossCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:


};
