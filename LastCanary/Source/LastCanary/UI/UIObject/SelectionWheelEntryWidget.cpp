#include "UI/UIObject/SelectionWheelEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void USelectionWheelEntryWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (LabelText)
    {
        LabelText->SetText(EntryData.DisplayName);
    }
    if (IconImage && EntryData.Icon)
    {
        IconImage->SetBrushFromTexture(EntryData.Icon);
    }
}

void USelectionWheelEntryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 부드러운 하이라이트 보간
    HighlightAlpha = FMath::FInterpTo(HighlightAlpha, TargetHighlightAlpha, InDeltaTime, 12.f);

    if (EntryBorder)
    {
        FLinearColor Base = FLinearColor(1, 1, 1, 0.3f);
        FLinearColor Hi = FLinearColor(1, 1, 1, 1.0f);
        EntryBorder->SetBrushColor(FMath::Lerp(Base, Hi, HighlightAlpha));
    }

    float Scale = FMath::Lerp(1.0f, 1.15f, HighlightAlpha);
    SetRenderScale(FVector2D(Scale, Scale));
}

void USelectionWheelEntryWidget::SetHighlighted_Implementation(bool bHighlighted)
{
    TargetHighlightAlpha = bHighlighted ? 1.f : 0.f;
}
