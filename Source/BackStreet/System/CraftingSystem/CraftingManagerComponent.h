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
	E_Spring				UMETA(DisplayName = "Spring"),
	E_Gear				UMETA(DisplayName = "Gear"),
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

	//UPROPERTY()
	//	int32 SuperMaterialIdx = 3

protected:
	UFUNCTION(BlueprintCallable)
		void InitCraftingManager();

//=======	Common Function		========================
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsItemEnough();

	UFUNCTION()
		bool ConsumeItem();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetIsSkillKeepingAvailable();

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
		bool UpgradeSkill(int32 SkillID, uint8 NewLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8>UpdateSkillUpgradeRequiredItemList(int32 SkillID, uint8 NewLevel);

//=======	Upgrade Weapon Function	====================	
	UFUNCTION(BlueprintCallable)
		bool UpgradeWeapon(TArray<uint8> NewLevelList);
		
	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<uint8> UpdateWeaponUpgradeRequiredItemList(TArray<uint8> NewLevelList);

	UFUNCTION(BlueprintCallable)
		bool GetIsStatLevelValid(TArray<uint8> NewLevelList);

//=======	CommonProperty		========================
public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		bool bIsSkillCreated = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		bool bIsDisplayingSkillListSet = false;

	//만능재료 사용 안함 = 0, 나사 = 1, 스프링 = 2, 기어 = 3
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
		EKeepMat KeptItem = EKeepMat::E_None; 

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TArray<uint8> RequiredItemList;

//=======	SkillSelectProperty		========================
public:
	//스킬 제작UI에 노출될 스킬 <SkillType, SkillID>
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TMap<ESkillType, int32> DisplayingSkillMap;

private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	TWeakObjectPtr<class AActor> OwnerActorRef;

	TWeakObjectPtr<class AMainCharacterBase> MainCharacterRef;

	TWeakObjectPtr<class UPlayerSkillManagerComponent> SkillManagerRef;

};
