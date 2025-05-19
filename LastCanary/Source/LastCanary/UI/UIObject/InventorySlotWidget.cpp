#include "UI/UIObject/InventorySlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "LastCanary.h"

void UInventorySlotWidget::SetItemData(const FBaseItemSlotData& InItemData, UDataTable* InItemDataTable)
{
	//ItemSlot = InItemSlot;
	ItemData = InItemData;
	ItemDataTable = InItemDataTable;
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

	FItemDataRow* Found = ItemDataTable->FindRow<FItemDataRow>(ItemData.ItemRowName, TEXT("UpdateSlotUI"));
	if (ItemNameText)
	{
		ItemNameText->SetText(FText::FromName(ItemData.ItemRowName));
	}

	if (ItemIconImage && Found->ItemIcon)
	{
		ItemIconImage->SetBrushFromTexture(Found->ItemIcon);
	}

	if (ItemQuantityText)
	{
		ItemQuantityText->SetText(FText::AsNumber(ItemData.Quantity));
	}
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

	if (InventoryComponent)
	{
		InventoryComponent->TrySwapItemSlots(SourceWidget->SlotIndex, this->SlotIndex);
		return true;
	}

	return false;
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
