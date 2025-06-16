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
#include "SaveGame/LCLocalPlayerSaveGame.h"

void UKeySettingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	/*
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
	*/
	SelectorMoveForward->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedMoveForward);
	SelectorMoveBackward->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedMoveBackward);
	SelectorMoveLeft->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedMoveLeft);
	SelectorMoveRight->OnKeySelected.AddUniqueDynamic(this, &UKeySettingWidget::OnKeySelectedMoveRight);
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
	InitialMappings();
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

void UKeySettingWidget::OnKeySelectedWalk(FInputChord SelectedKey)
{
	if (!SelectedKey.Key.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InValid Selected Key."));
		return;
	}

	FKey PressedKey = SelectedKey.Key;
	CurrentKeyWalk->SetText(FText::FromName(SelectedKey.Key.GetFName()));
	UpdateMappings("Walk", PressedKey);
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
	UpdateMappings("Sprint", PressedKey);
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
	UpdateMappings("Jump", PressedKey);
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
	UpdateMappings("Crouch", PressedKey);
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
	UpdateMappings("Interact", PressedKey);
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
	UpdateMappings("Aim", PressedKey);
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
	UpdateMappings("ThrowItem", PressedKey);
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
	UpdateMappings("VoiceChat", PressedKey);
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
	UpdateMappings("ChangeShootingMode", PressedKey);
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
	UpdateMappings("SelectQuickSlot1", PressedKey);
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
	UpdateMappings("SelectQuickSlot2", PressedKey);
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
	UpdateMappings("SelectQuickSlot3", PressedKey);
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
	UpdateMappings("SelectQuickSlot4", PressedKey);
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
	UpdateMappings("OpenPauseMenu", PressedKey);
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
	UpdateMappings("ExitDrone", PressedKey);
}



void UKeySettingWidget::RefreshMappings(const TArray<FEnhancedActionKeyMapping>& KeyMappings)
{
	UE_LOG(LogTemp, Warning, TEXT("RefreshMappings(const TArray<FEnhancedActionKeyMapping>& KeyMappings)"));
	for (const FEnhancedActionKeyMapping& Mapping : KeyMappings)
	{
		if (!Mapping.Action)
			continue;

		const FName MappingName = Mapping.GetMappingName();
		UE_LOG(LogTemp, Warning, TEXT("Mapping Name: %s"), *MappingName.ToString());

		if (MappingName == "MoveFront" && SelectorMoveForward)
		{
			SelectorMoveForward->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "MoveBack" && SelectorMoveBackward)
		{
			SelectorMoveBackward->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "MoveLeft" && SelectorMoveLeft)
		{
			SelectorMoveLeft->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "MoveRight" && SelectorMoveRight)
		{
			SelectorMoveRight->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "Jump" && SelectorJump)
		{
			SelectorJump->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "Crouch" && SelectorCrouch)
		{
			SelectorCrouch->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "Sprint" && SelectorSprint)
		{
			SelectorSprint->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "Aim" && SelectorAim)
		{
			SelectorAim->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "Interact" && SelectorInteract)
		{
			SelectorInteract->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "ThrowItem" && SelectorThrowItem)
		{
			SelectorThrowItem->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "VoiceChat" && SelectorVoice)
		{
			SelectorVoice->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "ChangeShootingMode" && SelectorChangeShootingMode)
		{
			SelectorChangeShootingMode->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "SelectQuickSlot1" && SelectorSelectQuickSlot1)
		{
			SelectorSelectQuickSlot1->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "SelectQuickSlot2" && SelectorSelectQuickSlot2)
		{
			SelectorSelectQuickSlot2->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "SelectQuickSlot3" && SelectorSelectQuickSlot3)
		{
			SelectorSelectQuickSlot3->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "SelectQuickSlot4" && SelectorSelectQuickSlot4)
		{
			SelectorSelectQuickSlot4->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "OpenPauseMenu" && SelectorOpenPauseMenu)
		{
			SelectorOpenPauseMenu->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "ExitDrone" && SelectorExitDrone)
		{
			SelectorExitDrone->SetSelectedKey(Mapping.Key);
		}
		else if (MappingName == "Walk" && SelectorWalk)
		{
			SelectorWalk->SetSelectedKey(Mapping.Key);
		}
		// 필요하면 더 추가 가능
	}
}

void UKeySettingWidget::UpdateMappings(FName MappingName, FKey Key)
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

	UE_LOG(LogTemp, Warning, TEXT("Update Mappings Mapping Name: %s"), *MappingName.ToString());

	FMapPlayerKeyArgs Args;
	Args.MappingName = MappingName;
	Args.NewKey = Key;
	Args.Slot = EPlayerMappableKeySlot::First;
	Args.bCreateMatchingSlotIfNeeded = true;
	Args.bDeferOnSettingsChangedBroadcast = false;

	FGameplayTagContainer FailureReason;
	UserSettings->MapPlayerKey(Args, FailureReason);

	UE_LOG(LogTemp, Warning, TEXT("Refresh"));
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Subsystem]()
		{
			const TArray<FEnhancedActionKeyMapping> Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();
			TArray<FSaveKeyMapping> SaveArray;

			for (const FEnhancedActionKeyMapping& Mapping : Mappings)
			{
				if (!Mapping.Action) continue;

				FSaveKeyMapping SaveMapping;
				SaveMapping.MappingName = Mapping.GetMappingName();
				SaveMapping.Key = Mapping.Key;

				SaveArray.Add(SaveMapping);
			}

			ULCLocalPlayerSaveGame::SaveKeyBindings(GetWorld(), SaveArray);

			RefreshMappings(Mappings);

		}, 0.1f, false);
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

	const TArray<FSaveKeyMapping> LoadedMappings = ULCLocalPlayerSaveGame::LoadKeyBindings(GetWorld());
	if (LoadedMappings.Num() == 0)
	{
		//return; 일단 지금 저장이 안되는 거 같아서 일단 막아놓았습니다.
	}

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!IsValid(UserSettings))
	{
		return;
	}

	FGameplayTagContainer FailureReason;

	// 현재 등록된 모든 키 매핑 가져오기
	const TArray<FEnhancedActionKeyMapping> AllMappings = Subsystem->GetAllPlayerMappableActionKeyMappings();

	for (const FSaveKeyMapping& SavedMapping : LoadedMappings)
	{
		// MappingName으로부터 매핑 찾기
		const FEnhancedActionKeyMapping* FoundMapping = AllMappings.FindByPredicate(
			[&](const FEnhancedActionKeyMapping& Mapping)
			{
				return Mapping.GetMappingName() == SavedMapping.MappingName;
			});

		if (!FoundMapping)
		{
			UE_LOG(LogTemp, Warning, TEXT("No mapping found for mapping name: %s"), *SavedMapping.MappingName.ToString());
			continue;
		}

		// MappingName을 가져와서 적용
		FMapPlayerKeyArgs Args;
		Args.MappingName = FoundMapping->GetMappingName();
		Args.NewKey = SavedMapping.Key;
		Args.Slot = EPlayerMappableKeySlot::First;
		Args.bCreateMatchingSlotIfNeeded = true;
		Args.bDeferOnSettingsChangedBroadcast = false;

		UserSettings->MapPlayerKey(Args, FailureReason);
	}

	UserSettings->SaveSettings();  // UserSettings 저장
	UserSettings->ApplySettings(); // 설정 적용

	// 최신 매핑 상태로 UI 갱신
	const TArray<FEnhancedActionKeyMapping> CurrentMappings = Subsystem->GetAllPlayerMappableActionKeyMappings();
	RefreshMappings(CurrentMappings);
}



void UKeySettingWidget::ResetKeyBindings()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC))
		return;

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!IsValid(LP))
		return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(Subsystem))
		return;

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!IsValid(UserSettings))
		return;

	const TArray<FEnhancedActionKeyMapping>& Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();

	FGameplayTagContainer DummyFailure;

	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (Mapping.Action)
		{
			FMapPlayerKeyArgs Args;
			Args.MappingName = Mapping.Action->GetFName();
			Args.Slot = EPlayerMappableKeySlot::First;  // 기본 슬롯 초기화
			Args.bCreateMatchingSlotIfNeeded = false;   // 초기화 시에는 false로 충분
			Args.bDeferOnSettingsChangedBroadcast = false;

			UserSettings->ResetAllPlayerKeysInRow(Args, DummyFailure);
		}
	}

	UserSettings->SaveSettings();
	UserSettings->ApplySettings();

	// UI 갱신
	const TArray<FEnhancedActionKeyMapping> UpdatedMappings = Subsystem->GetAllPlayerMappableActionKeyMappings();
	RefreshMappings(UpdatedMappings);
}
