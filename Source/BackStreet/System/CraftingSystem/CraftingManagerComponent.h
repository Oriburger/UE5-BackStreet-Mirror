// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "CraftingManagerComponent.generated.h"

UENUM(BlueprintType)
enum class EKeepMat : uint8
{
	E_None				UMETA(DisplayName = "None"),
	E_Screw				UMETA(DisplayName = "Screw"),
	E_Spring			UMETA(DisplayName = "Spring"),
	E_Gear				UMETA(DisplayName = "Gear"),
};

USTRUCT(BlueprintType)
struct FCraftingMaterialSet
{
	GENERATED_BODY()

public:
	//==== Static Proptery ====================

	//Stage type (entry, combat, time attack, boss, miniGame, gatcha etc..)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<int32> IDList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<int32> CountList;
	
	bool GetIsValidData() { return IDList.Num() == CountList.Num(); }
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UCraftingManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCraftingManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//=======	Common Function	& Property	================
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
		int32 MaxCraftSlotCount = 3; 

public:
	UFUNCTION(BlueprintCallable)
		void InitCraftingManager(class AMainCharacterBase* OwnerCharacterRef);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void OnCraftConfirmed(FSkillInfo SkillInfo);

	UFUNCTION(BlueprintCallable)
		bool UpgradeSkill(int32 SkillID, uint8 NewLevel);

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<int32> GetRandomCraftableSkillIdx(int32 MaxCount);

//=======	Getter Function		========================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<int32> GetCraftableIDList();


//=======	Ref Members		========================
protected:
	UPROPERTY(BlueprintReadWrite)	
		TArray<int32> CraftCandidateIDList;
	
	UPROPERTY(BlueprintReadWrite)
		TMap<int32, int32> CraftedSkillIDMap; //Info, Level

private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	TWeakObjectPtr<class AActor> OwnerActorRef;

	TWeakObjectPtr<class AMainCharacterBase> MainCharacterRef;

	TWeakObjectPtr<class USkillManagerComponentBase> SkillManagerRef;

};
