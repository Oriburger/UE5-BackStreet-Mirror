// Fill out your copyright notice in the Description page of Project Settings.


#include "BackStreetWidgetBase.h"
#include "../Global/BackStreetGameModeBase.h"
#include "../Character/MainCharacter/MainCharacterBase.h"
#include "../Character/Component/PlayerSkillManagerComponent.h"

void UBackStreetWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

AMainCharacterBase* UBackStreetWidgetBase::GetMainCharacterRef()
{
	ACharacter* character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (IsValid(character))
	{
		return Cast<AMainCharacterBase>(character);
	}
	return nullptr;
}

ABackStreetGameModeBase* UBackStreetWidgetBase::GetGamemodeRef()
{
	return Cast<ABackStreetGameModeBase>(GetWorld()->GetAuthGameMode());
}

UPlayerSkillManagerComponent* UBackStreetWidgetBase::GetSkillManagerRef()
{
	if (!IsValid(GetMainCharacterRef())) return nullptr;
	return GetMainCharacterRef()->SkillManagerComponent;
}