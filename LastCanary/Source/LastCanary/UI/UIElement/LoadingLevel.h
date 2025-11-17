#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "LoadingLevel.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class LASTCANARY_API ULoadingLevel : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 배경(백그라운드) 이미지 */
	UPROPERTY(meta = (BindWidget))
	UImage* BackGroundImage;

	/** Tool Tip */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelTipText;

	/** 카나리아 이미지 */
	UPROPERTY(meta = (BindWidget))
	UImage* CanaryImage;

	/** 교체할 이미지 텍스처들 */
	UPROPERTY(EditAnywhere, Category = "Loading Animation")
	TArray<UTexture2D*> LoadingTextures;

	/** 현재 인덱스 */
	int32 CurrentTextureIndex = 0;

	/** 타이머 핸들 */
	FTimerHandle TextureSwitchTimerHandle;

	/** 이미지 교체 함수 */
	void SwitchToNextTexture();
};
