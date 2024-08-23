// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Global/BackStreet.h"
#include "Blueprint/UserWidget.h"
#include "BackStreetWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UBackStreetWidgetBase : public UUserWidget
{
	GENERATED_BODY()

//--------- Basic --------------------------------
protected:
	virtual void NativeConstruct() override;

public:
	//IT MUST BE OVERRIDEN WITH ANIMATION
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void RemoveFromParentWithAnimation(class UWidgetAnimation* TargetAnimtaion, bool bIsReverse = false, float PlayRate = 1.0f);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void SetFocusToWidget(UBackStreetWidgetBase* WidgetToFocus, bool bIsGameAndUI, bool bIsGameOnly, bool bShowCursor);

//--------- Getter -------------------------------
protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
		class AMainCharacterBase* GetMainCharacterRef();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class ABackStreetGameModeBase* GetGamemodeRef();

	UFUNCTION(BlueprintCallable, BlueprintPure)
		class UPlayerSkillManagerComponent* GetSkillManagerRef();
};
