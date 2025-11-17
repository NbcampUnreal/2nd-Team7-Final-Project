#include "UI/UIElement/InGameHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"

void UInGameHUD::NativeConstruct()
{
	Super::NativeConstruct();

	if (InteractMessageText)
	{
		InteractMessageText->SetText(FText::FromString(TEXT("")));
	}
	SetVoiceKeyGuideText();
}

void UInGameHUD::NativeDestruct()
{
	Super::NativeDestruct();
}

void UInGameHUD::UpdateHPBar(float HealthPercent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercent);
	}
}

void UInGameHUD::UpdateStaminaBar(float StaminaPercent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(StaminaPercent);
	}
}

void UInGameHUD::PlayTakeDamageAnim()
{
	if (TakeDamageAnim)
	{
		PlayAnimation(TakeDamageAnim);
	}
}

void UInGameHUD::SetInteractMessage(const FString& Message)
{
	if (InteractMessageText)
	{
		InteractMessageText->SetText(FText::FromString(Message));
	}
}

void UInGameHUD::SetInteractMessageVisible(bool bVisible)
{
	if (InteractMessageText)
	{
		InteractMessageText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UInGameHUD::SetVoiceKeyGuideText()
{
	if (IA_Voice == nullptr)
	{
		return;
	}

	FString InteractKeyName = GetCurrentKeyNameForAction(IA_Voice);
	FString DisplayText = FString::Printf(TEXT("[ %s ]"), *InteractKeyName);

	VoiceKeyGuideText->SetText(FText::FromString(DisplayText));
}

FString UInGameHUD::GetCurrentKeyNameForAction(UInputAction* InputAction) const
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
			return Mapping.Key.GetDisplayName().ToString();
		}
	}
	return TEXT("V");
}