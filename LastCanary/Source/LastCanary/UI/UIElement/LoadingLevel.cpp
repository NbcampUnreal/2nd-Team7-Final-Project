#include "UI/UIElement/LoadingLevel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "Engine/World.h"

void ULoadingLevel::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기값 설정
	CurrentTextureIndex = 0;

	if (LoadingTextures.Num() > 0 && CanaryImage)
	{
		CanaryImage->SetBrushFromTexture(LoadingTextures[CurrentTextureIndex]);

		// 0.5초 간격으로 이미지 교체
		GetWorld()->GetTimerManager().SetTimer(
			TextureSwitchTimerHandle,
			this,
			&ULoadingLevel::SwitchToNextTexture,
			0.5f,
			true
		);
	}
}

void ULoadingLevel::NativeDestruct()
{
	Super::NativeDestruct();
	
	// 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TextureSwitchTimerHandle);
	}
}

void ULoadingLevel::SwitchToNextTexture()
{
	if (LoadingTextures.Num() == 0 || !CanaryImage)
	{
		return;
	}

	CurrentTextureIndex = (CurrentTextureIndex + 1) % LoadingTextures.Num();
	CanaryImage->SetBrushFromTexture(LoadingTextures[CurrentTextureIndex]);
}