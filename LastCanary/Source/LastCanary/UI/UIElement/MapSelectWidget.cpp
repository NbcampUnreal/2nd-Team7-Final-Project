#include "UI/UIElement/MapSelectWidget.h"
#include "Actor/LCGateActor.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

void UMapSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LeftButton)
	{
		LeftButton->OnClicked.AddUniqueDynamic(this, &UMapSelectWidget::OnLeftButtonClicked);
	}
	if (RightButton)
	{
		RightButton->OnClicked.AddUniqueDynamic(this, &UMapSelectWidget::OnRightButtonClicked);
	}
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddUniqueDynamic(this, &UMapSelectWidget::OnConfirmButtonClicked);
	}

	LoadMapData();
	UpdateMapDisplay();
}

void UMapSelectWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (LeftButton)
	{
		LeftButton->OnClicked.RemoveDynamic(this, &UMapSelectWidget::OnLeftButtonClicked);
	}
	if (RightButton)
	{
		RightButton->OnClicked.RemoveDynamic(this, &UMapSelectWidget::OnRightButtonClicked);
	}
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UMapSelectWidget::OnConfirmButtonClicked);
	}
}

void UMapSelectWidget::LoadMapData()
{
	if (MapDataTable == nullptr)
	{
		return;
	}

	static const FString ContextString(TEXT("MapDataContext"));
	MapDataTable->GetAllRows(ContextString, AllMapRows);

	CurrentIndex = 0;
}

void UMapSelectWidget::UpdateMapDisplay()
{
	if (AllMapRows.IsValidIndex(CurrentIndex) == false)
	{
		return;
	}

	const FMapDataRow* Row = AllMapRows[CurrentIndex];
	if (Row == nullptr)
	{
		return;
	}

	if (ThumbnailImage)
	{
		//ThumbnailImage->SetBrushFromTexture(Row->MapInfo.PreviewImage);
	}

	if (MapNameText)
	{
		MapNameText->SetText(FText::FromName(Row->MapInfo.MapName));
	}
}

int32 UMapSelectWidget::GetCurrentMapID() const
{
	return CurrentIndex;
}

void UMapSelectWidget::OnLeftButtonClicked()
{
	if (AllMapRows.Num() == 0)
	{
		return;
	}

	CurrentIndex = (CurrentIndex - 1 + AllMapRows.Num()) % AllMapRows.Num();
	UpdateMapDisplay();
}

void UMapSelectWidget::OnRightButtonClicked()
{
	if (AllMapRows.Num() == 0)
	{
		return;
	}

	CurrentIndex = (CurrentIndex + 1) % AllMapRows.Num();
	UpdateMapDisplay();
}

void UMapSelectWidget::OnConfirmButtonClicked()
{
	if (AllMapRows.IsValidIndex(CurrentIndex) == false)
	{
		return;
	}

	const FMapDataRow* SelectedRow = AllMapRows[CurrentIndex];
	if (SelectedRow == nullptr)
	{
		return;
	}

	if (GateActorInstance)
	{
		GateActorInstance->SetTargetMapID(SelectedRow->MapID);
		LOG_Frame_WARNING(TEXT("Map selected for Gate: %s (ID: %d)"),
			*SelectedRow->MapInfo.MapName.ToString(), SelectedRow->MapID);
	}

	ULCUIManager* UIManager = ResolveUIManager();
	if (UIManager)
	{
		UIManager->HideMapSelectPopup();
	}
}
