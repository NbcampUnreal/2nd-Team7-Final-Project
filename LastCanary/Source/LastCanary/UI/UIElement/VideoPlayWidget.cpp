#include "UI/UIElement/VideoPlayWidget.h"
#include "Components/Image.h"

#include "MediaPlayer.h"
#include "FileMediaSource.h"

void UVideoPlayWidget::NativeConstruct()
{
	if (MediaPlayer && MediaSource)
	{
		MediaPlayer->OpenSource(MediaSource);
		MediaPlayer->OnEndReached.AddUniqueDynamic(this, &UVideoPlayWidget::HandleVideoEnd);
	}
}

void UVideoPlayWidget::HandleVideoEnd()
{
	OnVideoEnded.ExecuteIfBound();
	RemoveFromParent();
}
