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

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DescriptionText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* RevealEntryAnim;

public:
	void InitWithEntry(const FResultRewardEntry& Entry);
};
