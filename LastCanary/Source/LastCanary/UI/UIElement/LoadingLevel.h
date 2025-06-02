#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "LoadingLevel.generated.h"

class UImage;
class UTextBlock;

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

};
