#include "KeySettingWidget.h"
#include "Components/InputKeySelector.h"

void UKeySettingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InputKeySelectorFrontKeyboard)
    {
        InputKeySelectorFrontKeyboard->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedFront);
    }
    if (InputKeySelectorBackKeyboard)
    {
        InputKeySelectorBackKeyboard->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedBack);
    }
    if (InputKeySelectorLeftKeyboard)
    {
        InputKeySelectorLeftKeyboard->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedLeft);
    }
    if (InputKeySelectorRightKeyboard)
    {
        InputKeySelectorRightKeyboard->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedRight);
    }
    if (InputKeySelectorJumpKeyboard)
    {
        InputKeySelectorJumpKeyboard->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedJump);
    }
    if (InputKeySelectorCrouchKeyboard)
    {
        InputKeySelectorCrouchKeyboard->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedCrouch);
    }
    if (InputKeySelectorWalkKeyboard)
    {
        InputKeySelectorWalkKeyboard->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedWalk);
    }
    if (InputKeySelectorRollKeyboard)
    {
        InputKeySelectorRollKeyboard->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedRoll);
    }
}

void UKeySettingWidget::OnKeySelectedFront(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Log, TEXT("[KeySetting] Front : %s"), *SelectedKey.Key.ToString());
}
void UKeySettingWidget::OnKeySelectedBack(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Log, TEXT("[KeySetting] Back : %s"), *SelectedKey.Key.ToString());
}
void UKeySettingWidget::OnKeySelectedLeft(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Log, TEXT("[KeySetting] Left : %s"), *SelectedKey.Key.ToString());
}
void UKeySettingWidget::OnKeySelectedRight(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Log, TEXT("[KeySetting] Right : %s"), *SelectedKey.Key.ToString());
}
void UKeySettingWidget::OnKeySelectedJump(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Log, TEXT("[KeySetting] Jump : %s"), *SelectedKey.Key.ToString());
}
void UKeySettingWidget::OnKeySelectedCrouch(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Log, TEXT("[KeySetting] Crouch : %s"), *SelectedKey.Key.ToString());
}
void UKeySettingWidget::OnKeySelectedWalk(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Log, TEXT("[KeySetting] Walk : %s"), *SelectedKey.Key.ToString());
}
void UKeySettingWidget::OnKeySelectedRoll(FInputChord SelectedKey)
{
    UE_LOG(LogTemp, Log, TEXT("[KeySetting] Roll : %s"), *SelectedKey.Key.ToString());
}