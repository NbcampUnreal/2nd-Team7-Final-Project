#include "UI/UIElement/SpectatorWidget.h"
#include "Components/TextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/PlayerController/LCInGamePlayerController.h"

void USpectatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PrevKeyText && IA_Move)
	{
		PrevKeyText->SetText(FText::FromString(GetDirectionalKeyName(IA_Move, -1.f))); // A 방향
	}
	if (NextKeyText && IA_Move)
	{
		NextKeyText->SetText(FText::FromString(GetDirectionalKeyName(IA_Move, +1.f))); // D 방향
	}
	if (PlayerName)
	{
		PlayerName->SetText(FText::FromString(FString::Printf(TEXT("%s"), TEXT("Default"))));
	}
	SetVoiceKeyGuideText();
}

FString USpectatorWidget::GetDirectionalKeyName(UInputAction* InputAction, float DirectionThreshold) const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(PC) == false)
	{
		return TEXT("Invalid");
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (IsValid(LP) == false)
	{
		return TEXT("Invalid");
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (IsValid(Subsystem) == false)
	{
		return TEXT("Invalid");
	}

	const TArray<FEnhancedActionKeyMapping> AllMappings = Subsystem->GetAllPlayerMappableActionKeyMappings();

	TArray<FString> MatchedKeys;

	for (const FEnhancedActionKeyMapping& Mapping : AllMappings)
	{
		if (Mapping.Action == nullptr || Mapping.Action != InputAction)
		{
			continue;
		}

		const FString KeyName = Mapping.Key.GetDisplayName().ToString();

		if (DirectionThreshold < 0.f && (KeyName == TEXT("A") || KeyName == TEXT("Left") || KeyName == TEXT("Q")))
		{
			MatchedKeys.AddUnique(KeyName);
		}
		else if (DirectionThreshold > 0.f && (KeyName == TEXT("D") || KeyName == TEXT("Right") || KeyName == TEXT("E")))
		{
			MatchedKeys.AddUnique(KeyName);
		}
	}

	if (MatchedKeys.Num() > 0)
	{
		return FString::Join(MatchedKeys, TEXT(" / "));
	}

	return TEXT("Unbound");
}

void USpectatorWidget::UpdatePlayerName(FString Name)
{
	if (PlayerName)
	{
		PlayerName->SetText(FText::FromString(Name));
	}
}

void USpectatorWidget::SetVoiceKeyGuideText()
{
	if (IA_Voice == nullptr)
	{
		return;
	}

	FString InteractKeyName = GetCurrentKeyNameForAction(IA_Voice);
	FString DisplayText = FString::Printf(TEXT("[ %s ]"), *InteractKeyName);

	VoiceKeyGuideText->SetText(FText::FromString(DisplayText));
}

FString USpectatorWidget::GetCurrentKeyNameForAction(UInputAction* InputAction) const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(PC) == false)
	{
		return TEXT("Invalid");
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (IsValid(LocalPlayer) == false)
	{
		return TEXT("Invalid");
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (IsValid(Subsystem) == false)
	{
		return TEXT("Invalid");
	}
	const TArray<FEnhancedActionKeyMapping> Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();

	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (Mapping.Action == InputAction)
		{
			LOG_Frame_WARNING(TEXT("Mapping Key = %s"), *Mapping.Key.GetDisplayName().ToString());
			return Mapping.Key.GetDisplayName().ToString();
		}
	}
	return TEXT("V");
}
