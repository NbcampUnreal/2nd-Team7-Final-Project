#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/UIElement/ToolbarInventoryWidget.h"
#include "UI/UIElement/BackpackInventoryWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"

#include "LastCanary.h"

void UInventoryMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Warning, TEXT("[InventoryMainWidget::NativeConstruct] 시작: %s"), *GetName());

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

    if (UToolbarInventoryComponent* ToolbarComp = Pawn->FindComponentByClass<UToolbarInventoryComponent>())
    {
        if (ToolbarWidget)
        {
            ToolbarWidget->SetInventoryComponent(ToolbarComp);
            UE_LOG(LogTemp, Warning, TEXT("[AutoInitializeWithPlayer] 툴바 위젯 연결 완료"));
        }
    }
    
    if (UBackpackInventoryComponent* BackpackComp = Pawn->FindComponentByClass<UBackpackInventoryComponent>())
    {
        if (BackpackWidget)
        {
            BackpackWidget->SetInventoryComponent(BackpackComp);
            UE_LOG(LogTemp, Warning, TEXT("[AutoInitializeWithPlayer] 백팩 위젯 연결 완료"));
        }
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
    if (bBackpackInventoryOpen)
    {
        // 가방 숨기기
        if (BackpackWidget)
        {
            BackpackWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
        bBackpackInventoryOpen = false;
    }
    else
    {
        // 가방 표시
        if (BackpackWidget)
        {
            BackpackWidget->SetVisibility(ESlateVisibility::Visible);
            BackpackWidget->RefreshInventoryUI();
        }
        bBackpackInventoryOpen = true;
    }
}

bool UInventoryMainWidget::IsBackpackInventoryOpen() const
{
    return bBackpackInventoryOpen;
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
