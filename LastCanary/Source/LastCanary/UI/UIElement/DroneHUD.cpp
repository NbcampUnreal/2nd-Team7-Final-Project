#include "UI/UIElement/DroneHUD.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"

void UDroneHUD::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateInstructionText(); // 위젯 표시 시 갱신
}

void UDroneHUD::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDroneHUD::UpdateDistanceDisplay(float Distance, float MaxDistance)
{
	const float Remaining = FMath::Max(0.0f, MaxDistance - Distance); // 여유 거리 계산

	// 거리 텍스트
	if (DistanceText)
	{
		FString DistanceString = FString::Printf(TEXT("%d cm"), FMath::RoundToInt(Remaining));
		DistanceText->SetText(FText::FromString(DistanceString));
	}

	if (DistanceProgressBar)
	{
		float Ratio = 1.0f - FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
		DistanceProgressBar->SetPercent(Ratio);

		// 명확한 HSV → RGB 계산 (초록~노랑~빨강)
		float Hue = FMath::Lerp(0.0f, 120.0f, Ratio); // 0 = 빨강, 120 = 초록
		FLinearColor HSVColor = FLinearColor::MakeFromHSV8((uint8)Hue, 255, 255);
		DistanceProgressBar->SetFillColorAndOpacity(HSVColor);
	}
}

void UDroneHUD::UpdateInstructionText()
{
	if (InstructionText == nullptr || IA_ExitDrone == nullptr)
	{
		return;
	}

	FString KeyName = GetCurrentKeyNameForAction(IA_ExitDrone);
	FString FinalText = FString::Printf(TEXT("Press [%s] to Exit Drone"), *KeyName);

	InstructionText->SetText(FText::FromString(FinalText));
}

FString UDroneHUD::GetCurrentKeyNameForAction(UInputAction* InputAction) const
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