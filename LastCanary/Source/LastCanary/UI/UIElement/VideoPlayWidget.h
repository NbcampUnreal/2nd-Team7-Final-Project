#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "VideoPlayWidget.generated.h"

class UMediaPlayer;
class UImage;
class UMediaTexture;
class UFileMediaSource;

UCLASS()
class LASTCANARY_API UVideoPlayWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UImage* VideoImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	UMediaPlayer* MediaPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	UFileMediaSource* MediaSource;

	DECLARE_DELEGATE(FOnVideoEnded)
	FOnVideoEnded OnVideoEnded;

	UFUNCTION()
	void HandleVideoEnd();
};
