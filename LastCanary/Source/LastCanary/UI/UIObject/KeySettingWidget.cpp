#include "KeySettingWidget.h"

#include "Components/InputKeySelector.h"
#include "Components/TextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputModifiers.h"

void UKeySettingWidget::NativeConstruct()
{
    Super::NativeConstruct();

    InitRow(TEXT("Move Forward"), IA_MoveForward, ActionLabelMoveForward, CurrentKeyMoveForward, SelectorMoveForward);
    InitRow(TEXT("Move Backward"), IA_MoveBackward, ActionLabelMoveBackward, CurrentKeyMoveBackward, SelectorMoveBackward);
    InitRow(TEXT("Strafe Left"), IA_StrafeLeft, ActionLabelStrafeLeft, CurrentKeyStrafeLeft, SelectorStrafeLeft);
    InitRow(TEXT("Strafe Right"), IA_StrafeRight, ActionLabelStrafeRight, CurrentKeyStrafeRight, SelectorStrafeRight);
    InitRow(TEXT("Walk"), IA_Walk, ActionLabelWalk, CurrentKeyWalk, SelectorWalk);
    InitRow(TEXT("Sprint"), IA_Sprint, ActionLabelSprint, CurrentKeySprint, SelectorSprint);
    InitRow(TEXT("Jump"), IA_Jump, ActionLabelJump, CurrentKeyJump, SelectorJump);
    InitRow(TEXT("Crouch"), IA_Crouch, ActionLabelCrouch, CurrentKeyCrouch, SelectorCrouch);
    InitRow(TEXT("Interact"), IA_Interact, ActionLabelInteract, CurrentKeyInteract, SelectorInteract);

    SelectorMoveForward->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedMoveForward);
    SelectorMoveBackward->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedMoveBackward);
    SelectorStrafeLeft->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedStrafeLeft);
    SelectorStrafeRight->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedStrafeRight);
    SelectorWalk->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedWalk);
    SelectorSprint->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedSprint);
    SelectorJump->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedJump);
    SelectorCrouch->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedCrouch);
    SelectorInteract->OnKeySelected.AddDynamic(this, &UKeySettingWidget::OnKeySelectedInteract);
}

void UKeySettingWidget::InitRow(const FString& DisplayName, UInputAction* Action, UTextBlock* Label, UTextBlock* KeyText, UInputKeySelector* Selector)
{
    if (!Action || !Label || !KeyText || !Selector || !MappingContext)
    {
        return;
    }

    Label->SetText(FText::FromString(DisplayName));
    FKey CurrentKey = GetMappedKey(Action);
    KeyText->SetText(FText::FromName(CurrentKey.GetFName()));
    Selector->SetSelectedKey(CurrentKey);
}

FKey UKeySettingWidget::GetMappedKey(UInputAction* Action) const
{
    if (!Action || !MappingContext)
    {
        return EKeys::Invalid;
    }

    for (const FEnhancedActionKeyMapping& Mapping : MappingContext->GetMappings())
    {
        if (Mapping.Action == Action)
        {
            return Mapping.Key;
        }
    }
    return EKeys::Invalid;
}

void UKeySettingWidget::SetMappedKey(UInputAction* Action, FKey NewKey)
{
    if (!Action || !MappingContext)
    {
        return;
    }

    MappingContext->UnmapKey(Action, GetMappedKey(Action));
    MappingContext->MapKey(Action, NewKey);
}

void UKeySettingWidget::OnKeySelectedMoveForward(FInputChord SelectedKey)
{
    SetMappedKey(IA_MoveForward, SelectedKey.Key);
    CurrentKeyMoveForward->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}
void UKeySettingWidget::OnKeySelectedMoveBackward(FInputChord SelectedKey)
{
    SetMappedKey(IA_MoveBackward, SelectedKey.Key);
    CurrentKeyMoveBackward->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}
void UKeySettingWidget::OnKeySelectedStrafeLeft(FInputChord SelectedKey)
{
    SetMappedKey(IA_StrafeLeft, SelectedKey.Key);
    CurrentKeyStrafeLeft->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}
void UKeySettingWidget::OnKeySelectedStrafeRight(FInputChord SelectedKey)
{
    SetMappedKey(IA_StrafeRight, SelectedKey.Key);
    CurrentKeyStrafeRight->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}
void UKeySettingWidget::OnKeySelectedWalk(FInputChord SelectedKey)
{
    SetMappedKey(IA_Walk, SelectedKey.Key);
    CurrentKeyWalk->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}
void UKeySettingWidget::OnKeySelectedSprint(FInputChord SelectedKey)
{
    SetMappedKey(IA_Sprint, SelectedKey.Key);
    CurrentKeySprint->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}
void UKeySettingWidget::OnKeySelectedJump(FInputChord SelectedKey)
{
    SetMappedKey(IA_Jump, SelectedKey.Key);
    CurrentKeyJump->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}
void UKeySettingWidget::OnKeySelectedCrouch(FInputChord SelectedKey)
{
    SetMappedKey(IA_Crouch, SelectedKey.Key);
    CurrentKeyCrouch->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}
void UKeySettingWidget::OnKeySelectedInteract(FInputChord SelectedKey)
{
    SetMappedKey(IA_Interact, SelectedKey.Key);
    CurrentKeyInteract->SetText(FText::FromName(SelectedKey.Key.GetFName()));
}