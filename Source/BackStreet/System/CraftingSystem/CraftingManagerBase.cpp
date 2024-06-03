// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerBase.h"
#include "../SkillSystem/SkillManagerBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Item/Weapon/WeaponBase.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
UCraftingManagerBase::UCraftingManagerBase()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> craftingRecipeTableFinder(TEXT("/Game/System/CraftingManager/Data/D_CraftingRecipe.D_CraftingRecipe"));
	static ConstructorHelpers::FObjectFinder<UDataTable> craftingSkillTableFinder(TEXT("/Game/System/CraftingManager/Data/D_CraftingSkillInfo.D_CraftingSkillInfo"));
	checkf(craftingRecipeTableFinder.Succeeded(), TEXT("CraftingRecipeTable class discovery failed."));
	checkf(craftingSkillTableFinder.Succeeded(), TEXT("CraftingSkillTable class discovery failed."));
	CraftingRecipeTable = craftingRecipeTableFinder.Object;
	CraftingSkillTable = craftingSkillTableFinder.Object;
}

void UCraftingManagerBase::InitCraftingManager(ABackStreetGameModeBase* NewGamemodeRef)
{
	if (!IsValid(NewGamemodeRef)) return;
	GamemodeRef = NewGamemodeRef;
}

void UCraftingManagerBase::AddSkill(int32 SkillID)
{
	AMainCharacterBase* mainCharacter = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!IsValid(mainCharacter->GetCurrentWeaponRef())) return;
	if (mainCharacter->GetCurrentWeaponRef()->GetWeaponStat().WeaponID == 0) return;

	FWeaponStateStruct weaponState = mainCharacter->GetCurrentWeaponRef()->GetWeaponState();

	if (!IsValid(CraftingSkillTable)) { UE_LOG(LogTemp, Log, TEXT("There's no CraftingSkillInfoTable")); return; }

	FString rowName = FString::FromInt(SkillID);
	FOwnerSkillInfoStruct* skillInfo = CraftingSkillTable->FindRow<FOwnerSkillInfoStruct>(FName(rowName), rowName);
	if (skillInfo == nullptr) return;

	switch (skillInfo->SkillType)
	{
	case ESkillType::E_Weapon0:
	{
		weaponState.SkillInfoMap[ESkillType::E_Weapon0] = *skillInfo;
	}
	case ESkillType::E_Weapon1:
	{
		weaponState.SkillInfoMap[ESkillType::E_Weapon1] = *skillInfo;
	}
	case ESkillType::E_Weapon2:
	{
		weaponState.SkillInfoMap[ESkillType::E_Weapon2] = *skillInfo;
	}
	default:
		break;
	}

	mainCharacter->GetCurrentWeaponRef()->SetWeaponState(weaponState);

	SkillUpdateDelegate.Broadcast(GamemodeRef.Get()->GetGlobalSkillManagerBaseRef()->GetSkillInfoStructByOwnerSkillInfo(*skillInfo));
}





