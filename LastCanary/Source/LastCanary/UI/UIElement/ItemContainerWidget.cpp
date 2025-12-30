#include "UI/UIElement/ItemContainerWidget.h"
#include "UI/UIObject/ItemContainerSlotWidget.h"
#include "UI/UIObject/InventorySlotWidget.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "Item/ItemContainer.h"
#include "Item/Component/ContainerInteractionComponent.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "Components/GridPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Blueprint/DragDropOperation.h"

#include "LastCanary.h"

void UItemContainerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!ContainerSlotWidgetClass)
    {
        LOG_Item_ERROR(TEXT("ContainerSlotWidgetClass is not set in blueprint!"));
    }
    LOG_Item_ERROR(TEXT("ContainerSlotWidgetClass is set in blueprint!"));

    // 닫기 버튼 바인딩
    if (CloseButton)
    {
        CloseButton->OnClicked.AddUniqueDynamic(this, &UItemContainerWidget::OnCloseButtonClicked);
    }

    // 데이터 테이블 로드
    LoadItemDataTable();
}

void UItemContainerWidget::NativeDestruct()
{
    Super::NativeDestruct();

    if (CloseButton)
    {
        CloseButton->OnClicked.RemoveDynamic(this, &UItemContainerWidget::OnCloseButtonClicked);
    }
}

bool UItemContainerWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    // 슬롯 위젯에서 드래그된 아이템인지 확인
    UInventorySlotWidget* SourceWidget = Cast<UInventorySlotWidget>(InOperation->Payload);
    if (!SourceWidget)
    {
        return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
    }

    // Default 아이템이면 드롭 불가
    if (SourceWidget->ItemData.ItemRowName.IsNone() ||
        SourceWidget->ItemData.ItemRowName == FName("Default"))
    {
        return false;
    }

    // TODO: 아이템 버리기 처리 (필요시 구현)
    LOG_Item_WARNING(TEXT("[ItemContainerWidget] 컨테이너 외부로 드롭 - 아이템 버리기 기능 미구현"));

    return true;
}

void UItemContainerWidget::SetupContainerWidget(AItemContainer* Container, const TArray<FContainerItemData>& ContainerItems)
{
    if (!Container)
    {
        LOG_Item_WARNING(TEXT("[ItemContainerWidget] Container is null"));
        return;
    }

    CurrentContainer = Container;
    CachedContainerItems = ContainerItems;

    // 컨테이너 이름 설정
    if (ContainerNameText)
    {
        ContainerNameText->SetText(Container->GetContainerName());
    }

    // UI 새로고침
    RefreshContainerUI();
}

void UItemContainerWidget::RefreshContainerUI()
{
    if (!ContainerSlotPanel)
    {
        LOG_Item_ERROR(TEXT("[ItemContainerWidget] ContainerSlotPanel이 바인딩되지 않음 - 블루프린트 확인 필요"));
        return;
    }

    if (!ContainerSlotWidgetClass)
    {
        LOG_Item_ERROR(TEXT("[ItemContainerWidget] ContainerSlotWidgetClass가 설정되지 않음 - 블루프린트에서 위젯 클래스 설정 필요"));
        return;
    }

    ClearContainerSlots();

    for (int32 i = 0; i < CachedContainerItems.Num(); ++i)
    {
        UItemContainerSlotWidget* SlotWidget = CreateContainerSlotWidget(i, CachedContainerItems[i]);
        if (!SlotWidget)
        {
            continue;
        }

        int32 Row = i / ColumnsPerRow;
        int32 Col = i % ColumnsPerRow;

        ContainerSlotPanel->AddChildToGrid(SlotWidget, Row, Col);
    }

    LOG_Item_WARNING(TEXT("[ItemContainerWidget] 컨테이너 UI 새로고침 완료: %d개 슬롯"), CachedContainerItems.Num());
}

void UItemContainerWidget::RefreshInventoryUI()
{
    RefreshContainerUI();
}

UItemContainerSlotWidget* UItemContainerWidget::CreateContainerSlotWidget(int32 SlotIndex, const FContainerItemData& ItemData)
{
    if (!ContainerSlotWidgetClass)
    {
        LOG_Item_ERROR(TEXT("[ItemContainerWidget] ContainerSlotWidgetClass가 설정되지 않음"));
        return nullptr;
    }

    UItemContainerSlotWidget* SlotWidget = CreateWidget<UItemContainerSlotWidget>(this, ContainerSlotWidgetClass);
    if (!SlotWidget)
    {
        LOG_Item_ERROR(TEXT("[ItemContainerWidget] 컨테이너 슬롯 위젯 생성 실패: %d"), SlotIndex);
        return nullptr;
    }

    SlotWidget->SetParentInventoryWidget(this);
    // 컨테이너 데이터를 베이스 슬롯 데이터로 변환
    FBaseItemSlotData BaseSlotData;
    BaseSlotData.ItemRowName = ItemData.ItemRowName;
    BaseSlotData.Quantity = ItemData.Quantity;
    BaseSlotData.Durability = ItemData.Durability;
    BaseSlotData.bIsValid = ItemData.IsValid();
    BaseSlotData.bIsEquipped = false;

    // 슬롯 데이터 설정
    SlotWidget->SetItemData(BaseSlotData, ItemDataTable);
    SlotWidget->ContainerSlotIndex = SlotIndex;
    SlotWidget->ContainerWidget = this;

    return SlotWidget;
}

void UItemContainerWidget::OnCloseButtonClicked()
{
    if (!CurrentContainer)
    {
        return;
    }

    APlayerController* PC = GetOwnerPlayerController();
    if (PC)
    {
        CurrentContainer->Server_CloseContainer(PC);
    }

    if (UInventoryMainWidget* MainWidget = GetTypedOuter<UInventoryMainWidget>())
    {
        MainWidget->HideContainerUI();
    }
    else
    {
        // 대안: 직접 숨김
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UItemContainerWidget::HandleItemMoveToContainer(UInventorySlotWidget* SourceWidget, int32 ContainerSlotIndex)
{
    if (!SourceWidget || !CurrentContainer)
    {
        return;
    }

    APlayerController* PC = GetOwnerPlayerController();
    if (!PC)
    {
        return;
    }

    ABaseCharacter* Character = GetOwningPlayerPawn<ABaseCharacter>();
    if (Character && Character->ContainerInteractionComponent)
    {
        Character->ContainerInteractionComponent->Server_MoveItemToContainer(
            CurrentContainer,
            SourceWidget->SlotIndex,
            ContainerSlotIndex,
            SourceWidget->ItemData.Quantity
        );
    }
}

void UItemContainerWidget::HandleItemMoveToPlayer(UItemContainerSlotWidget* SourceWidget, int32 PlayerSlotIndex)
{
    if (!SourceWidget || !CurrentContainer)
    {
        return;
    }

    APlayerController* PC = GetOwnerPlayerController();
    if (!PC)
    {
        return;
    }

    ABaseCharacter* Character = Cast<ABaseCharacter>(PC->GetPawn());
    if (!Character || !Character->ContainerInteractionComponent)
    {
        return;
    }

    Character->ContainerInteractionComponent->Server_MoveItemToPlayer(
        CurrentContainer,
        SourceWidget->ContainerSlotIndex,
        PlayerSlotIndex,
        SourceWidget->ItemData.Quantity
    );

    LOG_Item_WARNING(TEXT("[ItemContainerWidget] 컨테이너 → 플레이어 이동 요청: 슬롯 %d → %d"),
        SourceWidget->ContainerSlotIndex, PlayerSlotIndex);
}

void UItemContainerWidget::HandleContainerSlotSwap(UItemContainerSlotWidget* SourceWidget, int32 TargetSlotIndex)
{
    if (!SourceWidget || !CurrentContainer)
    {
        return;
    }

    // TODO : 컨테이너 슬롯 간 스왑은 필요시 구현
    LOG_Item_WARNING(TEXT("[ItemContainerWidget] 컨테이너 슬롯 스왑: %d <-> %d"),
        SourceWidget->ContainerSlotIndex, TargetSlotIndex);
}

void UItemContainerWidget::LoadItemDataTable()
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

void UItemContainerWidget::ClearContainerSlots()
{
    if (ContainerSlotPanel)
    {
        ContainerSlotPanel->ClearChildren();
    }
}

APlayerController* UItemContainerWidget::GetOwnerPlayerController() const
{
    return GetOwningPlayer();
}
