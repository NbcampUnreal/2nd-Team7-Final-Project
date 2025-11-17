#include "UI/UIObject/GraphicsOption.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/SizeBox.h"


void UGraphicsOption::NativeConstruct()
{
	Super::NativeConstruct();
	if (LeftButton)
	{
		LeftButton->OnClicked.AddUniqueDynamic(this, &UGraphicsOption::OnLeftButtonClicked);
	}
	if (RightButton)
	{
		RightButton->OnClicked.AddUniqueDynamic(this, &UGraphicsOption::OnRightButtonClicked);
	}
	UpdateUI();
}

void UGraphicsOption::NativeDestruct()
{
	Super::NativeDestruct();
	if (LeftButton)
	{
		LeftButton->OnClicked.RemoveDynamic(this, &UGraphicsOption::OnLeftButtonClicked);
	}
	if (RightButton)
	{
		RightButton->OnClicked.RemoveDynamic(this, &UGraphicsOption::OnRightButtonClicked);
	}
}

void UGraphicsOption::InitializeOption(FName InOptionKey, const TArray<FText>& InOptions, int32 DefaultIndex, const FText& LabelText)
{
	OptionKey = InOptionKey;
	Options = InOptions;
	CurrentIndex = FMath::Clamp(DefaultIndex, 0, Options.Num() - 1);

	if (TextBlock_Label && !LabelText.IsEmpty())
	{
		TextBlock_Label->SetText(LabelText);
	}

	UpdateUI();
}

FText UGraphicsOption::GetCurrentOption() const
{
	return Options.IsValidIndex(CurrentIndex) ? Options[CurrentIndex] : FText::GetEmpty();
}


void UGraphicsOption::UpdateUI()
{
	//텍스트 갱신
	if (CurrentOptionText && Options.IsValidIndex(CurrentIndex))
	{
		CurrentOptionText->SetText(Options[CurrentIndex]);
	}

	//버튼 활성화 상태 조정
	if (LeftButton)
	{
		LeftButton->SetIsEnabled(CurrentIndex > 0);
	}
	if (RightButton)
	{
		RightButton->SetIsEnabled(CurrentIndex < Options.Num() - 1);
	}

	//하이라이트 영역 갱신
	if (StepIndicators)
	{
		StepIndicators->ClearChildren();

		const int32 TotalSlots = 4;
		const int32 NumOptions = Options.Num();
		const int32 StartColumn = (TotalSlots - NumOptions) / 2;

		for (int32 i = 0; i < NumOptions; ++i)
		{
			UBorder* Dot = NewObject<UBorder>(this, UBorder::StaticClass());
			Dot->SetBrushColor(i == CurrentIndex ? FLinearColor::White : FLinearColor::Gray);

			// 배경 모양 추가 (기본 사각형)
			FSlateBrush Brush;
			Brush.TintColor = FSlateColor(i == CurrentIndex ? FLinearColor::White : FLinearColor::Gray);
			Brush.DrawAs = ESlateBrushDrawType::Box;
			Brush.Margin = FMargin(0);
			Brush.ImageSize = FVector2D(12.f, 12.f);
			Dot->SetBrush(Brush);

			USizeBox* SizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
			SizeBox->SetWidthOverride(50.f);
			SizeBox->SetHeightOverride(12.f);
			SizeBox->AddChild(Dot);

			if (UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(StepIndicators->AddChildToUniformGrid(SizeBox)))
			{
				GridSlot->SetRow(0);
				GridSlot->SetColumn(StartColumn + i);
				GridSlot->SetHorizontalAlignment(HAlign_Center);
				GridSlot->SetVerticalAlignment(VAlign_Center);
			}
		}
	}
}

void UGraphicsOption::OnLeftButtonClicked()
{
	if (CurrentIndex > 0)
	{
		--CurrentIndex;
		UpdateUI();
		OnOptionChanged.Broadcast(OptionKey, CurrentIndex);
	}
}

void UGraphicsOption::OnRightButtonClicked()
{
	if (CurrentIndex < Options.Num() - 1)
	{
		++CurrentIndex;
		UpdateUI();
		OnOptionChanged.Broadcast(OptionKey, CurrentIndex);
	}
}
