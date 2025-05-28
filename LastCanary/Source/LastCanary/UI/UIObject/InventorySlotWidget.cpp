#include "UI/UIObject/InventorySlotWidget.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
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
		{
			ItemNameText->SetText(FText::GetEmpty());
		}

		if (ItemIconImage)
		{
			ItemIconImage->SetVisibility(ESlateVisibility::Hidden);
		}

		if (ItemQuantityText)
		{
			ItemQuantityText->SetText(FText::GetEmpty());
		}

		UpdateBorderColor();
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

	UpdateBorderColor();
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

void UInventorySlotWidget::UpdateTooltipPosition()
{
	if (!ItemTooltipWidget || !ItemTooltipWidget->IsInViewport())
	{
		return;
	}

	FVector2D MousePosition;
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->GetMousePosition(MousePosition.X, MousePosition.Y);

		FVector2D TooltipPosition = MousePosition + FVector2D(15.0f, -50.0f);

		float MinY = 100.0f;
		TooltipPosition.Y = FMath::Max(TooltipPosition.Y, MinY);

		float MinX = 50.0f;
		TooltipPosition.X = FMath::Max(TooltipPosition.X, MinX);

		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);

			FVector2D EstimatedTooltipSize(230.0f, 230.0f);

			float MaxX = ViewportSize.X - EstimatedTooltipSize.X - 10.0f;
			float MaxY = ViewportSize.Y - EstimatedTooltipSize.Y - 10.0f;

			TooltipPosition.X = FMath::Clamp(TooltipPosition.X, MinX, MaxX);
			TooltipPosition.Y = FMath::Clamp(TooltipPosition.Y, MinY, MaxY);
		}

		ItemTooltipWidget->SetPositionInViewport(TooltipPosition);
	}
}

void UInventorySlotWidget::UpdateBorderColor()
{
	if (!SlotBorder)
	{
		// 보더가 바인딩되지 않았다면 로그만 남기고 리턴
		UE_LOG(LogTemp, VeryVerbose, TEXT("[UpdateBorderColor] SlotBorder가 바인딩되지 않음"));
		return;
	}

	FLinearColor TargetColor;

	if (ItemData.ItemRowName.IsNone())
	{
		// 빈 슬롯
		TargetColor = EmptyBorderColor;
	}
	else if (ItemData.bIsEquipped)
	{
		// 장착된 아이템
		TargetColor = EquippedBorderColor;
	}
	else
	{
		// 일반 아이템
		TargetColor = NormalBorderColor;
	}

	// 보더 색상 설정
	SlotBorder->SetBrushColor(TargetColor);
}

void UInventorySlotWidget::ShowTooltip()
{
	// 빈 슬롯이면 툴팁 표시하지 않음
	if (ItemData.ItemRowName.IsNone())
	{
		return;
	}

	// 이미 툴팁이 표시 중이면 스킵
	if (ItemTooltipWidget && ItemTooltipWidget->IsInViewport())
	{
		return;
	}

	// TooltipWidgetClass 확인
	if (!TooltipWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShowTooltip] TooltipWidgetClass가 설정되지 않음"));
		return;
	}

	// 툴팁 위젯 생성
	if (!ItemTooltipWidget)
	{
		ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(GetWorld(), TooltipWidgetClass);
		if (!ItemTooltipWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("[ShowTooltip] 툴팁 위젯 생성 실패"));
			return;
		}
	}

	if (ItemDataTable)
	{
		FItemDataRow* ItemRowData = ItemDataTable->FindRow<FItemDataRow>(ItemData.ItemRowName, TEXT("ShowTooltip"));
		if (ItemRowData)
		{
			ItemTooltipWidget->SetTooltipData(*ItemRowData, ItemData);
			ItemTooltipWidget->AddToViewport(10);

			UpdateTooltipPosition();

			GetWorld()->GetTimerManager().SetTimer(TooltipUpdateTimer,
				this, &UInventorySlotWidget::UpdateTooltipPosition,
				0.016f, true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShowTooltip] 아이템 데이터를 찾을 수 없음: %s"), *ItemData.ItemRowName.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShowTooltip] ItemDataTable이 NULL"));
	}
}

void UInventorySlotWidget::HideTooltip()
{

	if (TooltipUpdateTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TooltipUpdateTimer);
	}

	if (ItemTooltipWidget && ItemTooltipWidget->IsInViewport())
	{
		ItemTooltipWidget->RemoveFromParent();
	}
}

void UInventorySlotWidget::OnUseButtonClicked()
{
	// TODO : 슬롯을 클릭했을 때 사용, 버리기, 취소 버튼 등이 나오도록 작동
}
