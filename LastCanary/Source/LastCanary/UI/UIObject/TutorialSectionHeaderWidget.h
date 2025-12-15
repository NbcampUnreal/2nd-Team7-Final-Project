#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "TutorialSectionHeaderWidget.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class LASTCANARY_API UTutorialSectionHeaderWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SetTitle(const FText& InTitle);

protected:
	UPROPERTY(meta = (BindWidgetOptional)) 
	UTextBlock* TitleText;
};
