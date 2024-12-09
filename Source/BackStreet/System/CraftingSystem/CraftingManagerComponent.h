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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BACKSTREET_API UCraftingManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCraftingManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	UFUNCTION(BlueprintCallable)
		void InitCraftingManager();

//=======	Common Function		========================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsItemEnough();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsItemEnoughForCraft(int32 SkillID);

	UFUNCTION()
		bool ConsumeItem();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsSkillKeepingAvailable();

	UFUNCTION(BlueprintCallable)
		void KeepItem(EKeepMat NewKeptItem);

	UFUNCTION(BlueprintCallable)
		void UnKeepItem();

//=======	Select Skill Function	========================		
public:
	UFUNCTION(BlueprintCallable)
		bool AddSkill(int32 NewSkillID);

	UFUNCTION(BlueprintCallable)
		void SetDisplayingSkillList(); 

	UFUNCTION(BlueprintCallable)
		void KeepSkill(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		void UnkeepSkill(int32 SkillID);

//=======	Upgrade Skill Function	======================	
	UFUNCTION(BlueprintCallable)
		bool UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TMap<int32, int32> GetSkillCraftMaterialInfo(int32 SkillID);

	UFUNCTION(BlueprintCallable)
		TMap<int32, int32> UpdateSkillUpgradeRequiredItemList(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel);

	UFUNCTION(BlueprintCallable)
		TMap<int32, int32> UpdateSkillCraftRequiredItemList(int32 SkillID);

//=======	Upgrade Weapon Function	====================	
	UFUNCTION(BlueprintCallable)
		bool UpgradeWeapon(TArray<uint8> NewLevelList);
		
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> UpdateWeaponUpgradeRequiredItemList(TArray<uint8> NewLevelList);

	UFUNCTION(BlueprintCallable)
		bool GetIsStatLevelValid(TArray<uint8> NewLevelList);

//=======	Common Property		========================
public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		bool bIsSkillCreated = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		bool bIsDisplayingSkillListSet = false;

	//만능재료 사용 안함 = 0, 나사 = 1, 스프링 = 2, 기어 = 3
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		EKeepMat KeptItem = EKeepMat::E_None; 

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TMap<int32, int32> RequiredItemInfo;

//=======	SkillSelectProperty		========================
public:
	//스킬 제작UI에 노출될 스킬 <SkillType, SkillID>
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TMap<ESkillType, int32> DisplayingSkillMap;

private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	TWeakObjectPtr<class AActor> OwnerActorRef;

	TWeakObjectPtr<class AMainCharacterBase> MainCharacterRef;

	TWeakObjectPtr<class USkillManagerComponentBase> SkillManagerRef;

};
