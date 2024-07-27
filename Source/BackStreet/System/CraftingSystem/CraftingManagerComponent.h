// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Global/BackStreet.h"
#include "Components/ActorComponent.h"
#include "CraftingManagerComponent.generated.h"


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
	UFUNCTION()
	void InitCraftingManager();
//SkillUpgrade == SU
//WeaponUpgrade == WU
//=======	Common Function		========================
public:
	UFUNCTION()
		bool IsMatEnough();

	UFUNCTION()
		bool ConsumeMat();

//=======	Select Skill Function	========================		
public:
	UFUNCTION()
		bool AddSkill(int32 NewSkillID);

	UFUNCTION()
		void SetDisplayingSkillList();

//=======	Upgrade Skill Function	======================	
	UFUNCTION(BlueprintCallable)
		bool UpgradeSkill(int32 SkillID, uint8 NewLevel);

	UFUNCTION()
		TArray<uint8>UpdateRequiredMatForSU(int32 SkillID, uint8 NewLevel);
//=======	Upgrade Weapon Function	====================	
	UFUNCTION(BlueprintCallable)
		bool UpgradeWeapon(TArray<uint8> NewLevelList);
		
	UFUNCTION()
		TArray<uint8>UpdateRequiredMatForWU(TArray<uint8> NewLevelList);
//=======	CommonProperty		========================
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsSkillCreated = false;

	//만능재료 사용 안함 = 0, 나사 = 1, 스프링 = 2, 기어 = 3
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		uint8 KeepMatIdx = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<uint8> RequiredMatList;
//=======	SkillSelectProperty		========================
public:
	//스킬 제작UI에 노출될 스킬 <SkillType, SkillID>
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<ESkillType, int32>DisplayingSkillMap;

private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	TWeakObjectPtr<class ACraftBoxBase> OwnerActorRef;

	TWeakObjectPtr<class AMainCharacterBase> MainCharacterRef;

	TWeakObjectPtr<class USkillManagerComponent> SkillManagerRef;

};
