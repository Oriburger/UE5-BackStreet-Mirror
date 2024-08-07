// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "SkillManagerComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateEquiped, int32, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegateSkillUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateSkillActivated, int32, SkillID);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BACKSTREET_API USkillManagerComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USkillManagerComponentBase();

public:
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillUpdated OnSkillUpdated;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable)
		FDelegateSkillActivated OnSkillActivated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//======= Basic function ========================
protected:
	virtual void InitSkillManager();

	virtual void InitSkillMap();

public:
	virtual bool TrySkill(int32 SkillID);

	virtual bool AddSkill(int32 SkillID);

	virtual bool RemoveSkill(int32 SkillID);

	virtual bool UpgradeSkill(int32 SkillID, uint8 NewLevel);

	virtual void ClearAllSkill();

//======== Getter ============================
public:
	virtual bool IsSkillValid(int32 SkillID);

	virtual ASkillBase* GetOwnSkillBase(int32 SkillID);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct GetSkillInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		ESkillType GetSkillTypeInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStatStruct GetOwnSkillStat(int32 SkillID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FSkillStateStruct GetOwnSkillState(int32 SkillID);

//======= DataTable =============================================================================
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gamplay|Data")
		UDataTable* SkillStatTable;

//======= Ref =========
protected:
	//GameMode Soft Ref
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	//owner character ref
	TWeakObjectPtr<class ACharacterBase> OwnerCharacterRef;
};