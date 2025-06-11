#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/UIElement/ToolbarInventoryWidget.h"
#include "UI/UIElement/BackpackInventoryWidget.h"
#include "Character/BaseCharacter.h"
#include "DataType/BaseItemSlotData.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"

#include "Components/TextBlock.h"

#include "LastCanary.h"

void UInventoryMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AutoInitializeWithPlayer();

	ShowToolbarOnly();
}

void UInventoryMainWidget::AutoInitializeWithPlayer()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoInitializeWithPlayer] PlayerController 또는 Pawn이 NULL"));
		return;
	}

	APawn* Pawn = PC->GetPawn();

	UToolbarInventoryComponent* ToolbarComp = Pawn->FindComponentByClass<UToolbarInventoryComponent>();
	if (!ToolbarComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoInitializeWithPlayer] 툴바 컴포넌트가 없습니다."));
		return;
	}

	if (ToolbarWidget)
	{
		ToolbarWidget->SetInventoryComponent(ToolbarComp);
	}

	if (BackpackWidget)
	{
		BackpackWidget->SetInventoryComponent(ToolbarComp);
	}
}

void UInventoryMainWidget::ShowToolbarOnly()
{
	if (ToolbarWidget)
	{
		ToolbarWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (BackpackWidget)
	{
		BackpackWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	bBackpackInventoryOpen = false;
}

void UInventoryMainWidget::ToggleBackpackInventory()
{
	if (!BackpackWidget)
	{
		LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] 가방 위젯이 없습니다."));

		return;
	}

	if (bBackpackInventoryOpen)
	{
		BackpackWidget->SetVisibility(ESlateVisibility::Collapsed);
		bBackpackInventoryOpen = false;
		LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] 가방 인벤토리 닫기"));
	}
	else
	{
		BackpackWidget->SetVisibility(ESlateVisibility::Visible);
		BackpackWidget->RefreshInventoryUI();
		bBackpackInventoryOpen = true;
		LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] 가방 인벤토리 열기"));
	}
}

bool UInventoryMainWidget::IsBackpackInventoryOpen() const
{
	return bBackpackInventoryOpen;
}

void UInventoryMainWidget::ShowToolbarSlotItemText(const FText& ItemName)
{
	if (!SlotItemText) return;

	SlotItemText->SetText(FText::Format(NSLOCTEXT("UI", "CurrentSlotItem", "현재 아이템 : {0}"), ItemName));
	SlotItemText->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().ClearTimer(SlotItemTextTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(SlotItemTextTimerHandle, this, &UInventoryMainWidget::HideToolbarSlotItemText, 1.5f, false);
}

void UInventoryMainWidget::HideToolbarSlotItemText()
{
	if (SlotItemText)
	{
		SlotItemText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryMainWidget::RefreshInventory()
{
	if (ToolbarWidget)
	{
		ToolbarWidget->RefreshInventoryUI();
	}

	if (BackpackWidget)
	{
		BackpackWidget->RefreshInventoryUI();
	}
}

