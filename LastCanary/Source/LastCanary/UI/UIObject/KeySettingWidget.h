#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InputCoreTypes.h"
#include "KeySettingWidget.generated.h"

/**
 *
 */
class UInputKeySelector;
class UTextBlock;
UCLASS()
class LASTCANARY_API UKeySettingWidget : public ULCUserWidgetBase
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InputKeySelectorFrontKeyboard;
    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InputKeySelectorBackKeyboard;
    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InputKeySelectorLeftKeyboard;
    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InputKeySelectorRightKeyboard;
    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InputKeySelectorJumpKeyboard;
    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InputKeySelectorCrouchKeyboard;
    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InputKeySelectorWalkKeyboard;
    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InputKeySelectorRollKeyboard;

    // TODO : 필요하다면 Gamepad용도 추가
    // ...

    UFUNCTION()
    void OnKeySelectedFront(FInputChord SelectedKey);
    UFUNCTION()
    void OnKeySelectedBack(FInputChord SelectedKey);
    UFUNCTION()
    void OnKeySelectedLeft(FInputChord SelectedKey);
    UFUNCTION()
    void OnKeySelectedRight(FInputChord SelectedKey);
    UFUNCTION()
    void OnKeySelectedJump(FInputChord SelectedKey);
    UFUNCTION()
    void OnKeySelectedCrouch(FInputChord SelectedKey);
    UFUNCTION()
    void OnKeySelectedWalk(FInputChord SelectedKey);
    UFUNCTION()
    void OnKeySelectedRoll(FInputChord SelectedKey);
};
