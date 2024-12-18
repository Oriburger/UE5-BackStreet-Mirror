// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerComponent.h"
#include "CraftBoxBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/SkillManagerComponentBase.h"
#include "../../Character/Component/WeaponComponentBase.h"
#include "../../Character/Component/ItemInventoryComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

	CraftCandidateIDList = GetRandomCraftableSkillIdx(MaxCraftSlotCount);
}

bool UCraftingManagerComponent::UpgradeSkill(int32 SkillID, uint8 NewLevel)
{
	if (!SkillManagerRef.IsValid()) return false;
	return true;
}

TArray<int32> UCraftingManagerComponent::GetRandomCraftableSkillIdx(int32 MaxCount)
{
	TArray<int32> result = TArray<int32>();
	TArray<int32> craftableIDList = GetCraftableIDList();

	UE_LOG(LogTemp, Warning, TEXT("UCraftingManagerComponent::GetRandomCraftableSkillIdx(%d) #1 - %d"), MaxCount, craftableIDList.Num());

	while (result.Num() < MaxCount)
	{
		if (craftableIDList.IsEmpty()) break;

		const int32 targetIdx = UKismetMathLibrary::RandomInteger(craftableIDList.Num());
		result.Add(craftableIDList[targetIdx]);
		craftableIDList.RemoveAt(targetIdx);
	}
	UE_LOG(LogTemp, Warning, TEXT("UCraftingManagerComponent::GetRandomCraftableSkillIdx(%d) = %d"), MaxCount, result.Num());
	return result;
}

TArray<int32> UCraftingManagerComponent::GetCraftableIDList()
{
	if (!SkillManagerRef.IsValid()) return {};
	return SkillManagerRef.Get()->GetCraftableIDList();
}
