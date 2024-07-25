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
	//��ų ����UI���� ���� �Ǿ��� ��ų ����Ʈ�� ���
	TArray<int32> displayedSkillIDList;
	DisplayingSkillMap.GenerateValueArray(displayedSkillIDList);
	for (int32 skillID : displayedSkillIDList)
	{
		SkillManagerRef->DisplayedSkillList.Add(skillID);
	}
	//��ų �ߺ� �߰� ����
	bIsSkillCreated = true;
	//��ų �߰�
	SkillManagerRef->AddSkill(NewSkillID);
	return true;
}

void UCraftingManagerComponent::SetDisplayingSkillList()
{
	DisplayingSkillMap.Reset();
	TArray<ESkillType> skillTypeList = MainCharacterRef->WeaponComponent->WeaponStat.SkillTypeList;
	TArray<int32> keepSkillList = SkillManagerRef->KeepSkillList;
	//�� �Ǿ��ִ� ��ų Ÿ���� �ִ��� Ȯ��
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

	//�� �ȵǾ� �ִ� ��ų Ÿ���� �������� ����ġ ����Ͽ� ����
	for (ESkillType skillType : skillTypeList)
	{
		if(DisplayingSkillMap.Contains(skillType)) continue;
		else 
		{
			TMap<ESkillType, FSkillListContainer>obtainableSkillMap = SkillManagerRef->GetObtainableSkillMap();
			checkf(obtainableSkillMap.Contains(skillType), TEXT("ObtainableSkillMap is something wrong"));
			//����ġ ���� ����
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
