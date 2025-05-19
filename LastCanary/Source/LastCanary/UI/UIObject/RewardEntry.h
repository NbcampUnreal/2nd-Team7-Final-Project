#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "UI/UIElement/ResultMenu.h"
#include "RewardEntry.generated.h"

/**
 * 
 */
class UTextBlock;

UCLASS()
class LASTCANARY_API URewardEntry : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	void InitWithEntry(const FResultRewardEntry& Entry);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;
};
