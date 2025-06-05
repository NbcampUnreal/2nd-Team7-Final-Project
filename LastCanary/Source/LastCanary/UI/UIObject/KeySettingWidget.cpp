#include "KeySettingWidget.h"

#include "Components/InputKeySelector.h"
#include "Components/TextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputModifiers.h"
#include "Character/BasePlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputTriggers.h"
#include "InputCoreTypes.h"
#include "UserSettings/EnhancedInputUserSettings.h"

void UKeySettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitRow(TEXT("Move Forward"), IA_MoveForward, ActionLabelMoveForward, CurrentKeyMoveForward, SelectorMoveForward);
	InitRow(TEXT("Move Backward"), IA_MoveBackward, ActionLabelMoveBackward, CurrentKeyMoveBackward, SelectorMoveBackward);
	InitRow(TEXT("Move Left"), IA_MoveLeft, ActionLabelMoveLeft, CurrentKeyMoveLeft, SelectorMoveLeft);
	InitRow(TEXT("Move Right"), IA_MoveRight, ActionLabelMoveRight, CurrentKeyMoveRight, SelectorMoveRight);
	//InitRow(TEXT("Strafe Left"), IA_StrafeLeft, ActionLabelStrafeLeft, CurrentKeyStrafeLeft, SelectorStrafeLeft);
	//InitRow(TEXT("Strafe Right"), IA_StrafeRight, ActionLabelStrafeRight, CurrentKeyStrafeRight, SelectorStrafeRight);
	InitRow(TEXT("Walk"), IA_Walk, ActionLabelWalk, CurrentKeyWalk, SelectorWalk);
	InitRow(TEXT("Sprint"), IA_Sprint, ActionLabelSprint, CurrentKeySprint, SelectorSprint);
	InitRow(TEXT("Jump"), IA_Jump, ActionLabelJump, CurrentKeyJump, SelectorJump);
	InitRow(TEXT("Crouch"), IA_Crouch, ActionLabelCrouch, CurrentKeyCrouch, SelectorCrouch);
	InitRow(TEXT("Interact"), IA_Interact, ActionLabelInteract, CurrentKeyInteract, SelectorInteract);
	InitRow(TEXT("Aim"), IA_Aim, ActionLabelAim, CurrentKeyAim, SelectorAim);
	InitRow(TEXT("ThrowItem"), IA_ThrowItem, ActionLabelThrowItem, CurrentKeyThrowItem, SelectorThrowItem);
	InitRow(TEXT("Voice"), IA_Voice, ActionLabelVoice, CurrentKeyVoice, SelectorVoice);
	InitRow(TEXT("ChangeShootingMode"), IA_ChangeShootingMode, ActionLabelChangeShootingMode, CurrentKeyChangeShootingMode, SelectorChangeShootingMode);
	InitRow(TEXT("SelectQuickSlot1"), IA_SelectQuickSlot1, ActionLabelSelectQuickSlot1, CurrentKeySelectQuickSlot1, SelectorSelectQuickSlot1);
	InitRow(TEXT("SelectQuickSlot2"), IA_SelectQuickSlot2, ActionLabelSelectQuickSlot2, CurrentKeySelectQuickSlot2, SelectorSelectQuickSlot2);
	InitRow(TEXT("SelectQuickSlot3"), IA_SelectQuickSlot3, ActionLabelSelectQuickSlot3, CurrentKeySelectQuickSlot3, SelectorSelectQuickSlot3);
	InitRow(TEXT("SelectQuickSlot4"), IA_SelectQuickSlot4, ActionLabelSelectQuickSlot4, CurrentKeySelectQuickSlot4, SelectorSelectQuickSlot4);
	InitRow(TEXT("OpenPauseMenu"), IA_OpenPauseMenu, ActionLabelOpenPauseMenu, CurrentKeyOpenPauseMenu, SelectorOpenPauseMenu);
	InitRow(TEXT("OpenExitDrone"), IA_OpenExitDrone, ActionLabelExitDrone, CurrentKeyExitDrone, SelectorExitDrone);

	SelectorMoveForward->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedMoveForward);
	SelectorMoveBackward->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedMoveBackward);
	SelectorMoveLeft->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedMoveLeft);
	SelectorMoveRight->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedMoveRight);
	//SelectorStrafeLeft->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedStrafeLeft);
	//SelectorStrafeRight->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedStrafeRight);
	SelectorWalk->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedWalk);
	SelectorSprint->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedSprint);
	SelectorJump->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedJump);
	SelectorCrouch->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedCrouch);
	SelectorInteract->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedInteract);
	SelectorAim->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedAim);
	SelectorThrowItem->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedThrowItem);
	SelectorVoice->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedVoice);
	SelectorChangeShootingMode->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedChangeShootingMode);
	SelectorSelectQuickSlot1->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedChangeQuickSlot1);
	SelectorSelectQuickSlot2->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedChangeQuickSlot2);
	SelectorSelectQuickSlot3->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedChangeQuickSlot3);
	SelectorSelectQuickSlot4->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedChangeQuickSlot4);
	SelectorOpenPauseMenu->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedOpenPauseMenu);
	SelectorExitDrone->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedExitDrone);
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
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyMoveForward->SetText(FText::FromName(SelectedKey.Key.GetFName()));

	UpdateMappings("MoveFront", PressedKey);
}
void UKeySettingWidget::OnKeySelectedMoveBackward(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyMoveBackward->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("MoveBack", PressedKey);
}
void UKeySettingWidget::OnKeySelectedMoveLeft(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyMoveLeft->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("MoveLeft", PressedKey);
}
void UKeySettingWidget::OnKeySelectedMoveRight(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyMoveRight->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("MoveRight", PressedKey);
}
//void UKeySettingWidget::OnKeySelectedStrafeLeft(FInputChord SelectedKey)
//{
//	if (!SelectedKey.Key.IsValid())
//	{
//		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
//		return;
//	}
//
//	FKey PressedKey = SelectedKey.Key;
//	CurrentKeyStrafeLeft->SetText(FText::FromName(SelectedKey.Key.GetFName()));
//	UpdateMappings("DroneStrafeLeft_Keyboard", PressedKey);
//}
//void UKeySettingWidget::OnKeySelectedStrafeRight(FInputChord SelectedKey)
//{
//	if (!SelectedKey.Key.IsValid())
//	{
//		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
//		return;
//	}
//
//	FKey PressedKey = SelectedKey.Key;
//	CurrentKeyStrafeRight->SetText(FText::FromName(SelectedKey.Key.GetFName()));
//	UpdateMappings("DroneStrafeRight_Keyboard", PressedKey);
//}
void UKeySettingWidget::OnKeySelectedWalk(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyWalk->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("Walk_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedSprint(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeySprint->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("Sprint_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedJump(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyJump->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("Jump_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedCrouch(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyCrouch->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("Crouch_Keyboard", PressedKey);
}

void UKeySettingWidget::OnKeySelectedInteract(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyInteract->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("Interact_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedAim(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyAim->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("Aim_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedThrowItem(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyThrowItem->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("ThrowItem_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedVoice(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyVoice->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("VoiceChat_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedChangeShootingMode(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyChangeShootingMode->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("ChangeShootingMode_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedChangeQuickSlot1(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeySelectQuickSlot1->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("SelectQuickSlot1_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedChangeQuickSlot2(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeySelectQuickSlot2->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("SelectQuickSlot2_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedChangeQuickSlot3(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeySelectQuickSlot3->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("SelectQuickSlot3_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedChangeQuickSlot4(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeySelectQuickSlot4->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("SelectQuickSlot4_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedOpenPauseMenu(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyOpenPauseMenu->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("OpenPauseMenu_Keyboard", PressedKey);
}
void UKeySettingWidget::OnKeySelectedExitDrone(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyExitDrone->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("ExitDrone_Keyboard", PressedKey);
}



void UKeySettingWidget::RefreshMappings(const TArray<FEnhancedActionKeyMapping>& KeyMappings)
{
	for (const FEnhancedActionKeyMapping& Mapping : KeyMappings)
	{
		const FText MappingName = Mapping.GetDisplayName();
		UE_LOG(LogTemp, Warning, TEXT("Mapping Name: %s"), *Mapping.GetDisplayName().ToString());
		if (MappingName.EqualTo(FText::FromString("MoveFront")) && SelectorMoveForward)
		{
			SelectorMoveForward->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("MoveBack")) && SelectorMoveBackward)
		{
			SelectorMoveBackward->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("MoveLeft")) && SelectorMoveLeft)
		{
			SelectorMoveLeft->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("MoveRight")) && SelectorMoveRight)
		{
			SelectorMoveRight->SetSelectedKey(Mapping.Key);
		}

		/*if (MappingName.EqualTo(FText::FromString("DroneStrafeLeft_Keyboard")) && SelectorStrafeLeft)
		{
			SelectorStrafeLeft->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("DroneStrafeRight_Keyboard")) && SelectorStrafeRight)
		{
			SelectorStrafeRight->SetSelectedKey(Mapping.Key);
		}*/

		if (MappingName.EqualTo(FText::FromString("Jump_Keyboard")) && SelectorJump)
		{
			SelectorJump->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("Crouch_Keyboard")) && SelectorCrouch)
		{
			SelectorCrouch->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("Sprint_Keyboard")) && SelectorSprint)
		{
			SelectorSprint->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("Aim_Keyboard")) && SelectorSprint)
		{
			SelectorAim->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("Interact_Keyboard")) && SelectorInteract)
		{
			SelectorInteract->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("ThrowItem_Keyboard")) && SelectorThrowItem)
		{
			SelectorThrowItem->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("VoiceChat_Keyboard")) && SelectorVoice)
		{
			SelectorVoice->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("ChangeShootingMode_Keyboard")) && SelectorChangeShootingMode)
		{
			SelectorChangeShootingMode->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("SelectQuickSlot1_Keyboard")) && SelectorSelectQuickSlot1)
		{
			SelectorSelectQuickSlot1->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("SelectQuickSlot2_Keyboard")) && SelectorSelectQuickSlot2)
		{
			SelectorSelectQuickSlot2->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("SelectQuickSlot3_Keyboard")) && SelectorSelectQuickSlot3)
		{
			SelectorSelectQuickSlot3->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("SelectQuickSlot4_Keyboard")) && SelectorSelectQuickSlot4)
		{
			SelectorSelectQuickSlot4->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("OpenPauseMenu_Keyboard")) && SelectorOpenPauseMenu)
		{
			SelectorOpenPauseMenu->SetSelectedKey(Mapping.Key);
		}

		if (MappingName.EqualTo(FText::FromString("ExitDrone_Keyboard")) && SelectorExitDrone)
		{
			SelectorExitDrone->SetSelectedKey(Mapping.Key);
		}
		// ... 다른 키들도 여기에 추가 가능
	}
}

void UKeySettingWidget::UpdateMappings(FName DisplayName, FKey Key)
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC))
	{
		return;
	}
	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!IsValid(LP))
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(Subsystem))
	{
		return;
	}
	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!IsValid(UserSettings))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Update Mappings Mapping Name: %s"), *DisplayName.ToString());
	FMapPlayerKeyArgs Args;
	Args.MappingName = DisplayName;
	Args.NewKey = Key;
	Args.Slot = EPlayerMappableKeySlot::First;
	Args.bCreateMatchingSlotIfNeeded = true;
	Args.bDeferOnSettingsChangedBroadcast = false;

	FGameplayTagContainer FailureReason;
	UserSettings->MapPlayerKey(Args, FailureReason);


	UE_LOG(LogTemp, Warning, TEXT("Refresh"));
	// 딜레이 후 키 매핑 가져오기
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Subsystem]()
		{
			const TArray<FEnhancedActionKeyMapping> Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();
			RefreshMappings(Mappings);

		}, 0.2f, false);
}


void UKeySettingWidget::InitialMappings()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC))
	{
		return;
	}
	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!IsValid(LP))
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(Subsystem))
	{
		return;
	}
	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!IsValid(UserSettings))
	{
		return;
	}
	const TArray<FEnhancedActionKeyMapping> Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();
	RefreshMappings(Mappings);
}