// Fill out your copyright notice in the Description page of Project Settings.


#include "CraftingManagerComponent.h"
#include "CraftBoxBase.h"
#include "../../Global/BackStreetGameModeBase.h"
#include "../../Character/MainCharacter/MainCharacterBase.h"
#include "../../Character/Component/SkillManagerComponent.h"
#include "../../Character/Component/WeaponComponentBase.h"

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
	OwnerActorRef = Cast<ACraftBoxBase>(GetOwner());
	MainCharacterRef = Cast<AMainCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	SkillManagerRef = MainCharacterRef->SkillManagerComponent;
	bIsSkillCreated = false;
}

bool UCraftingManagerComponent::AddSkill(int32 NewSkillID)
{	
	//스킬 제작UI에서 노출 되었던 스킬 리스트에 등록
	TArray<int32> displayedSkillIDList;
	DisplayingSkillMap.GenerateValueArray(displayedSkillIDList);
	for (int32 skillID : displayedSkillIDList)
	{
		SkillManagerRef->DisplayedSkillList.Add(skillID);
	}
	//스킬 중복 추가 방지
	bIsSkillCreated = true;
	//스킬 추가
	SkillManagerRef->AddSkill(NewSkillID);
	return true;
}

void UCraftingManagerComponent::SetDisplayingSkillList()
{
	DisplayingSkillMap.Reset();
	TArray<ESkillType> skillTypeList = MainCharacterRef->WeaponComponent->WeaponStat.SkillTypeList;
	TArray<int32> keepSkillList = SkillManagerRef->KeepSkillList;
	//찜 되어있는 스킬 타입이 있는지 확인
	for (int32 keepSkill : keepSkillList)
	{
		for (ESkillType skillType : skillTypeList)
		{
			if (SkillManagerRef->GetSkillInfo(keepSkill).SkillWeaponStruct.SkillType == skillType)
			{
				DisplayingSkillMap.Add(skillType, keepSkill);
			}
		}
	}

	//찜 안되어 있는 스킬 타입은 랜덤으로 가중치 고려하여 선택
	for (ESkillType skillType : skillTypeList)
	{
		if(DisplayingSkillMap.Contains(skillType)) continue;
		else 
		{
			TMap<ESkillType, FSkillListContainer>obtainableSkillMap = SkillManagerRef->GetObtainableSkillMap();
			checkf(obtainableSkillMap.Contains(skillType), TEXT("ObtainableSkillMap is something wrong"));
			//가중치 랜덤 로직
			int32 totalWeight = 0;
			for (int32 skillID : obtainableSkillMap[skillType].SkillIDList)
			{
				totalWeight += SkillManagerRef->GetSkillInfo(skillID).SkillWeaponStruct.RandomWeight;
			}
			int32 pivot = FMath::RandRange(0, totalWeight);
			int32 tempWeight = 0;
			for (int32 skillID : obtainableSkillMap[skillType].SkillIDList)
			{
				tempWeight += SkillManagerRef->GetSkillInfo(skillID).SkillWeaponStruct.RandomWeight;
				if (tempWeight >= pivot)
				{
					DisplayingSkillMap.Add(skillType, skillID);
					break;
				}
			}
		}
	}
}
