// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Global/BackStreet.h"
#include "CommonButtonBase.h"
#include "CommonButtonBaseForKeyboard.generated.h"

/**
 * 
 */
UCLASS()
class BACKSTREET_API UCommonButtonBaseForKeyboard : public UCommonButtonBase
{
	GENERATED_BODY()

public:
#if PLATFORM_DESKTOP || WITH_EDITOR
    virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;

    virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

    virtual void NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
#endif

};
