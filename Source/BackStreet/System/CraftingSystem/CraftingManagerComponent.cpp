// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerComponent.h"
#include "CraftBoxBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/SkillManagerComponentBase.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"

UCraftingManagerComponent::UCraftingManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UCraftingManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	InitCraftingManager();
}

void UCraftingManagerComponent::InitCraftingManager()
{
	GameModeRef = Cast<ABackStreetGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	OwnerActorRef = GetOwner();
	OwnerActorRef->Tags.Add("CraftingBox");
	MainCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	SkillManagerRef = MainCharacterRef.IsValid() ? MainCharacterRef->SkillManagerComponent : nullptr;
	
	bIsSkillCreated = false;
	bIsDisplayingSkillListSet = false;

	UE_LOG(LogTemp, Warning, TEXT("UCraftingManagerComponent::InitCraftingManager()"));
}

bool UCraftingManagerComponent::GetIsItemEnough()
{
	return true;
}

bool UCraftingManagerComponent::GetIsItemEnoughForCraft(int32 SkillID)
{
	return true;
}

bool UCraftingManagerComponent::ConsumeItem()
{
	return true;
}

bool UCraftingManagerComponent::GetIsSkillKeepingAvailable()
{	
	return true;
}

void UCraftingManagerComponent::KeepItem(EKeepMat NewKeptItem)
{
}

void UCraftingManagerComponent::UnKeepItem()
{
}

bool UCraftingManagerComponent::AddSkill(int32 NewSkillID)
{	
	return true;
}

void UCraftingManagerComponent::SetDisplayingSkillList()
{

}

void UCraftingManagerComponent::KeepSkill(int32 SkillID)
{
}

void UCraftingManagerComponent::UnkeepSkill(int32 SkillID)
{
}

bool UCraftingManagerComponent::UpgradeSkill(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	return true;
}

TMap<int32, int32> UCraftingManagerComponent::GetSkillCraftMaterialInfo(int32 SkillID)
{
	return TMap<int32, int32>();
}

TMap<int32, int32> UCraftingManagerComponent::UpdateSkillUpgradeRequiredItemList(int32 SkillID, ESkillUpgradeType UpgradeTarget, uint8 NewLevel)
{
	return TMap<int32, int32>();
}

TMap<int32, int32> UCraftingManagerComponent::UpdateSkillCraftRequiredItemList(int32 SkillID)
{
	return TMap<int32, int32>();
}

bool UCraftingManagerComponent::UpgradeWeapon(TArray<uint8> NewLevelList)
{
	return false;
}

TArray<uint8> UCraftingManagerComponent::UpdateWeaponUpgradeRequiredItemList(TArray<uint8> NewLevelList)
{
	return {};
}

bool UCraftingManagerComponent::GetIsStatLevelValid(TArray<uint8> NewLevelList)
{
	return true;
}
