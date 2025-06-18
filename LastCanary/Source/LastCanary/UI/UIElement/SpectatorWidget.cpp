#include "UI/UIElement/SpectatorWidget.h"
#include "Components/TextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "Kismet/GameplayStatics.h"

void USpectatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PrevKeyText && IA_SpectatePrev)
	{
		PrevKeyText->SetText(FText::FromString(GetCurrentKeyNameForAction(IA_SpectatePrev)));
	}

	if (NextKeyText && IA_SpectateNext)
	{
		NextKeyText->SetText(FText::FromString(GetCurrentKeyNameForAction(IA_SpectateNext)));
	}
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
			return Mapping.Key.GetDisplayName().ToString();
		}
	}

	return TEXT("Unbound");
}
