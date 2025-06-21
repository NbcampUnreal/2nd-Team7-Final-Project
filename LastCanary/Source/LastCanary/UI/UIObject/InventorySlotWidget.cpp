#include "UI/UIObject/InventorySlotWidget.h"
#include "UI/UIObject/InventoryWidgetBase.h"
#include "UI/UIObject/BackpackSlotWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/InventoryUtility.h"
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
		if (ItemIconImage)
		{
			LOG_Item_WARNING(TEXT("[InventorySlotWidget::UpdateSlotUI] ItemIconImage!!!"));

			//ItemIconImage->SetVisibility(ESlateVisibility::Hidden);
		}

		UpdateBorderImage();
		return;
	}

	FItemDataRow* ItemRowData = ItemDataTable->FindRow<FItemDataRow>(ItemData.ItemRowName, TEXT("UpdateSlotUI"));
	if (!ItemRowData)
	{
		LOG_Item_WARNING(TEXT("[InventorySlotWidget::UpdateSlotUI] ItemData not found for: %s"),
			*ItemData.ItemRowName.ToString());
		return;
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

	UpdateBorderImage();
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
	if (UInventoryUtility::IsDefaultItem(ItemData.ItemRowName, InventoryComponent ? InventoryComponent->GetInventoryConfig() : nullptr))
	{
		OutOperation = nullptr;
		return;
	}

	if (ItemData.Quantity <= 0)
	{
		OutOperation = nullptr;
		return;
	}

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

	// ⭐ 소스가 가방 슬롯인지 확인
	UBackpackSlotWidget* SourceBackpackWidget = Cast<UBackpackSlotWidget>(SourceWidget);
	if (SourceBackpackWidget)
	{
		// 가방에서 툴바로 아이템 이동
		UToolbarInventoryComponent* ToolbarInventory = Cast<UToolbarInventoryComponent>(InventoryComponent);
		if (!ToolbarInventory)
		{
			LOG_Item_WARNING(TEXT("[InventorySlotWidget::NativeOnDrop] ToolbarInventoryComponent 캐스팅 실패"));
			return false;
		}

		bool bSuccess = ToolbarInventory->TryMoveBackpackItemToToolbar(
			SourceBackpackWidget->BackpackSlotIndex,
			this->SlotIndex
		);

		LOG_Item_WARNING(TEXT("[InventorySlotWidget::NativeOnDrop] 가방->툴바 이동 결과: %s (from backpack %d to toolbar %d)"),
			bSuccess ? TEXT("성공") : TEXT("실패"),
			SourceBackpackWidget->BackpackSlotIndex,
			this->SlotIndex);

		return bSuccess;
	}
	else
	{
		// 기존 툴바 슬롯 간 스왑 로직
		bool bSuccess = InventoryComponent->TrySwapItemSlots(SourceWidget->SlotIndex, this->SlotIndex);

		LOG_Item_WARNING(TEXT("[InventorySlotWidget::NativeOnDrop] 툴바 슬롯 교체 결과: %s (from %d to %d)"),
			bSuccess ? TEXT("성공") : TEXT("실패"), SourceWidget->SlotIndex, this->SlotIndex);

		return bSuccess;
	}
}

void UInventorySlotWidget::SetParentInventoryWidget(UInventoryWidgetBase* InParentWidget)
{
	ParentInventoryWidget = InParentWidget;
}

void UInventorySlotWidget::UpdateBorderImage()
{
	if (!SlotBorder)
	{
		LOG_Item_WARNING(TEXT("[UInventorySlotWidget::UpdateBorderImage] SlotBorder가 바인딩되지 않음"));
		return;
	}

	UTexture2D* TargetTexture = nullptr;

	if (ItemData.bIsEquipped)
    {
        TargetTexture = EquippedBorderTexture;
    }
    else
    {
        TargetTexture = NormalBorderTexture;
    }

	if (TargetTexture)
	{
		FSlateBrush NewBrush;
		NewBrush.SetResourceObject(TargetTexture);
		NewBrush.ImageSize = FVector2D(64.f, 64.f); // 필요한 크기로 설정

		SlotBorder->SetBrush(NewBrush);
	}
	else
	{
		LOG_Item_WARNING(TEXT("[UInventorySlotWidget::UpdateBorderImage] TargetTexture가 설정되지 않음"));
	}
}

void UInventorySlotWidget::ShowTooltip()
{
	if (!ParentInventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySlotWidget::ShowTooltip] ParentInventoryWidget이 설정되지 않음"));
		return;
	}

	// ⭐ 부모 위젯의 공유 툴팁 사용
	ParentInventoryWidget->ShowTooltipForSlot(ItemData, this);
}

void UInventorySlotWidget::HideTooltip()
{
	if (!ParentInventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventorySlotWidget::HideTooltip] ParentInventoryWidget이 설정되지 않음"));
		return;
	}

	// ⭐ 부모 위젯의 공유 툴팁 숨김
	ParentInventoryWidget->HideTooltip();
}

void UInventorySlotWidget::OnUseButtonClicked()
{
	// TODO : 슬롯을 클릭했을 때 사용, 버리기, 취소 버튼 등이 나오도록 작동

	if (!InventoryComponent || ItemData.ItemRowName.IsNone())
	{
		return;
	}

	// 가방 아이템인지 확인
	if (const FItemDataRow* ItemRowData = ItemDataTable->FindRow<FItemDataRow>(ItemData.ItemRowName, TEXT("OnUseButtonClicked")))
	{
		static const FGameplayTag BackpackTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Backpack"));
		if (ItemRowData->ItemType.MatchesTag(BackpackTag))
		{
			// 가방 사용 (인벤토리 열기)
			UE_LOG(LogTemp, Warning, TEXT("[OnUseButtonClicked] 가방 사용 - 인벤토리 열기"));

			if (ULCGameInstanceSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				if (ULCUIManager* UIManager = Subsystem->GetUIManager())
				{
					UIManager->ToggleInventory();
				}
			}
			return;
		}
	}
}

bool UInventorySlotWidget::IsDefaultItem(FName ItemRowName) const
{
	return ItemRowName == FName("Default");
}