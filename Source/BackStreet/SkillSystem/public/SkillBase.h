
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkillBase.generated.h"

UCLASS()
class BACKSTREET_API ASkillBase : public AActor
{
	GENERATED_BODY()

	public:
	// Sets default values for this actor's properties
	ASkillBase();

	UFUNCTION(BlueprintImplementableEvent)
	void InitSkill(AActor* Causer, class ACharacterBase* Target, ESkillGrade SkillGrade);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};