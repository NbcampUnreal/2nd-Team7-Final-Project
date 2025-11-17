#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "TutorialRowWidget.generated.h"

/**
 *
 */
class UImage;
class UTextBlock;
UCLASS()
class LASTCANARY_API UTutorialRowWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void Setup(class UTexture2D* InIcon, const FText& InText);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* IconImage;
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* BodyText;
};

