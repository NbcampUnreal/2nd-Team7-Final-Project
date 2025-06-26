#include "UI/UIElement/AnimatedMouseCursor.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnimatedMouseCursor::UAnimatedMouseCursor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAnimatedMouseCursor::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentFrameIndex = 0;

	if (CursorFrames.Num() > 0 && CursorImage)
	{
		CursorImage->SetBrushFromTexture(CursorFrames[CurrentFrameIndex]);

		// 일정 시간마다 이미지 교체
		GetWorld()->GetTimerManager().SetTimer(
			FrameSwitchTimerHandle,
			this,
			&UAnimatedMouseCursor::SwitchToNextFrame,
			FrameInterval,
			true
		);
	}
}

void UAnimatedMouseCursor::NativeDestruct()
{
	Super::NativeDestruct();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FrameSwitchTimerHandle);
	}
}

void UAnimatedMouseCursor::SwitchToNextFrame()
{
	if (CursorFrames.Num() == 0 || !CursorImage)
	{
		return;
	}

	CurrentFrameIndex = (CurrentFrameIndex + 1) % CursorFrames.Num();
	CursorImage->SetBrushFromTexture(CursorFrames[CurrentFrameIndex]);
}
