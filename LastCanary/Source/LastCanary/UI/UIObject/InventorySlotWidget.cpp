#include "UI/UIObject/InventorySlotWidget.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "LastCanary.h"

void UInventorySlotWidget::SetItemData(const FBaseItemSlotData& InItemData, UDataTable* InItemDataTable)
{
	ItemData = InItemData;
	ItemDataTable = InItemDataTable;

	if (!ItemDataTable)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
				{
					ItemDataTable = Subsystem->GetItemDataTable();
				}
			}
		}
	}

	UpdateSlotUI();
}

void UInventorySlotWidget::SetInventoryComponent(UInventoryComponentBase* InInventoryComponent)
{
	if (!InInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[InventorySlotWidget::SetInventoryComponent] InInventoryComponent is null!"));
		return;
	}

	InventoryComponent = InInventoryComponent;
}

void UInventorySlotWidget::UpdateSlotUI()
{
	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[InventorySlotWidget::UpdateSlotUI] ItemDataTable is null"));
		return;
	}

	if (ItemData.ItemRowName.IsNone())
	{
		if (ItemNameText)
			ItemNameText->SetText(FText::GetEmpty());

		if (ItemIconImage)
			ItemIconImage->SetVisibility(ESlateVisibility::Hidden);

		if (ItemQuantityText)
			ItemQuantityText->SetText(FText::GetEmpty());

		return;
	}

	FItemDataRow* ItemRowData = ItemDataTable->FindRow<FItemDataRow>(ItemData.ItemRowName, TEXT("UpdateSlotUI"));
	if (!ItemRowData)
	{
		LOG_Item_WARNING(TEXT("[InventorySlotWidget::UpdateSlotUI] ItemData not found for: %s"),
			*ItemData.ItemRowName.ToString());
		return;
	}

	// UI 업데이트
	if (ItemNameText)
	{
		ItemNameText->SetText(FText::FromName(ItemRowData->ItemName));
	}

	if (ItemIconImage)
	{
		if (ItemRowData->ItemIcon)
		{
			ItemIconImage->SetBrushFromTexture(ItemRowData->ItemIcon);
			ItemIconImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIconImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (ItemQuantityText)
	{
		if (ItemData.Quantity > 1)
		{
			ItemQuantityText->SetText(FText::AsNumber(ItemData.Quantity));
			ItemQuantityText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemQuantityText->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	LOG_Item_WARNING(TEXT("[InventorySlotWidget::UpdateSlotUI] UI 업데이트 완료: %s (Q:%d)"),
		*ItemData.ItemRowName.ToString(), ItemData.Quantity);
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// 만일 블루프린트에서 효과를 추가하려면
	// OnSlotHovered();

	ShowTooltip();
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	// 만일 블루프린트에서 효과를 추가하려면
	// OnSlotUnhovered();

	HideTooltip();
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// 만일 블루프린트에서 효과를 추가하려면
	// OnSlotDragStarted();

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();

	UInventorySlotWidget* DragVisual = CreateWidget<UInventorySlotWidget>(GetWorld(), GetClass());
	if (DragVisual)
	{
		DragVisual->SetItemData(ItemData, ItemDataTable);
		DragVisual->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	DragOp->DefaultDragVisual = DragVisual;
	DragOp->Payload = this;

	OutOperation = DragOp;
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventorySlotWidget* SourceWidget = Cast<UInventorySlotWidget>(InOperation->Payload);
	if (!SourceWidget || SourceWidget == this)
	{
		return false;
	}

	if (!InventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[InventorySlotWidget::NativeOnDrop] InventoryComponent is null"));
		return false;
	}

	// 슬롯 교체 시도
	bool bSuccess = InventoryComponent->TrySwapItemSlots(SourceWidget->SlotIndex, this->SlotIndex);

	LOG_Item_WARNING(TEXT("[InventorySlotWidget::NativeOnDrop] 슬롯 교체 결과: %s (from %d to %d)"),
		bSuccess ? TEXT("성공") : TEXT("실패"), SourceWidget->SlotIndex, this->SlotIndex);

	return bSuccess;
}

void UInventorySlotWidget::ShowTooltip()
{
	if (TooltipWidgetClass && !ItemTooltipWidget)
	{
		ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(GetWorld(), TooltipWidgetClass);
	}
	if (ItemTooltipWidget)
	{
		//ItemTooltipWidget->SetTooltipData(ItemData, );
		ItemTooltipWidget->AddToViewport();
	}
}

void UInventorySlotWidget::HideTooltip()
{
	// TODO : UI 매니저에서 종료 지시
}

void UInventorySlotWidget::OnUseButtonClicked()
{
	// TODO : 슬롯을 클릭했을 때 사용, 버리기, 취소 버튼 등이 나오도록 작동
}
