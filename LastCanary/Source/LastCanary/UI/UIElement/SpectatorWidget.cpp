#include "UI/UIElement/SpectatorWidget.h"
#include "Components/TextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

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
