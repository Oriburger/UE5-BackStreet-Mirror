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
//=======	Common Function		========================
public:
	//UFUNCTION()
	//	void GetRequiredMat();
//=======	Select Skill Function	========================		
public:
	UFUNCTION()
		bool AddSkill(int32 NewSkillID);

	UFUNCTION()
		void SetDisplayingSkillList();


//=======	Getter	================================
//=======	Data	==================================
public:
   	//스킬 제작UI에 노출될 스킬 <SkillType, SkillID>
	TMap<ESkillType, int32>DisplayingSkillMap;
//=======	Property		==============================
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		bool bIsSkillCreated = false;

private:
	TWeakObjectPtr<class ABackStreetGameModeBase> GameModeRef;

	TWeakObjectPtr<class ACraftBoxBase> OwnerActorRef;

	TWeakObjectPtr<class AMainCharacterBase> MainCharacterRef;

	TWeakObjectPtr<class USkillManagerComponent> SkillManagerRef;

};
