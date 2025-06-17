#include "UI/UIElement/DroneHUD.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UDroneHUD::NativeConstruct()
{
	Super::NativeConstruct();
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
		FString DistanceString = FString::Printf(TEXT("%dm"), FMath::RoundToInt(Remaining));
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
