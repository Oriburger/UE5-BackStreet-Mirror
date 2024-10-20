// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonButtonBaseForKeyboard.h"

#if PLATFORM_DESKTOP || WITH_EDITOR

void UCommonButtonBaseForKeyboard::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
    if (InFocusEvent.GetCause() != EFocusCause::Mouse)
    {
        SetSelectedInternal(true);

        Super::NativeOnAddedToFocusPath(InFocusEvent);
    }
}

void UCommonButtonBaseForKeyboard::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    if (InFocusEvent.GetCause() != EFocusCause::Mouse)
    {
        SetSelectedInternal(false);

        Super::NativeOnRemovedFromFocusPath(InFocusEvent);
    }
}

void UCommonButtonBaseForKeyboard::NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent)
{
    if (InFocusEvent.GetCause() != EFocusCause::Mouse)
    {
        Super::NativeOnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
    }
    else
    {
        SetKeyboardFocus();
    }
}

#endif