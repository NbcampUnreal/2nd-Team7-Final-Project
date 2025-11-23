#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Popup/SelectionWheelWidget.h"
#include "SelectionWheelEntryWidget.generated.h"

struct FSelectionWheelEntry;

class UImage;
class UTextBlock;
class UBorder;

UCLASS()
class LASTCANARY_API USelectionWheelEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Selection Wheel")
    FSelectionWheelEntry EntryData;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* IconImage;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* LabelText;

    UPROPERTY(meta = (BindWidgetOptional))
    UBorder* EntryBorder;

    UFUNCTION(BlueprintNativeEvent, Category = "Selection Wheel")
    void SetHighlighted(bool bHighlighted);
    virtual void SetHighlighted_Implementation(bool bHighlighted);

protected:
    virtual void NativePreConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    float HighlightAlpha = 0.f;
    float TargetHighlightAlpha = 0.f;
};
