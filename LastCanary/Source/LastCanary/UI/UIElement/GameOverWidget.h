#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "GameOverWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UButton;
UCLASS()
class LASTCANARY_API UGameOverWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* GameOverText;
};