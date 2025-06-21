#include "Inventory/InventoryUIController.h"
#include "Inventory/InventoryComponentBase.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "LastCanary.h"

UInventoryUIController::UInventoryUIController()
{
    OwnerInventory = nullptr;
    CachedUIManager = nullptr;
    CachedInventoryWidget = nullptr;
}

void UInventoryUIController::Initialize(UInventoryComponentBase* InOwnerInventory)
{
    OwnerInventory = InOwnerInventory;
    CachedUIManager = nullptr;
    CachedInventoryWidget = nullptr;

    if (OwnerInventory)
    {
        LOG_Item_WARNING(TEXT("[InventoryUIController::Initialize] UI 컨트롤러 초기화 완료"));
    }
}

void UInventoryUIController::RefreshInventoryUI()
{
    if (!IsLocalPlayer())
    {
        return;
    }

    UInventoryMainWidget* InventoryWidget = GetInventoryWidget();
    if (InventoryWidget)
    {
        InventoryWidget->RefreshInventory();
        LOG_Item_WARNING(TEXT("[InventoryUIController::RefreshInventoryUI] 인벤토리 UI 새로고침 완료"));
    }
}

void UInventoryUIController::UpdateSlotUI(int32 SlotIndex)
{
    if (!IsLocalPlayer())
    {
        return;
    }

    // 개별 슬롯 업데이트는 전체 새로고침으로 처리
    // 필요시 향후 최적화 가능
    RefreshInventoryUI();

    LOG_Item_WARNING(TEXT("[InventoryUIController::UpdateSlotUI] 슬롯 %d UI 업데이트"), SlotIndex);
}

void UInventoryUIController::UpdateEquippedItemText(const FText& ItemName)
{
    if (!IsLocalPlayer())
    {
        return;
    }

    UInventoryMainWidget* InventoryWidget = GetInventoryWidget();
    if (InventoryWidget)
    {
        InventoryWidget->ShowToolbarSlotItemText(ItemName);
        LOG_Item_WARNING(TEXT("[InventoryUIController::UpdateEquippedItemText] 장착 아이템 텍스트 업데이트: %s"),
            *ItemName.ToString());
    }
}

void UInventoryUIController::UpdateWeightUI(float CurrentWeight)
{
    if (!IsLocalPlayer())
    {
        return;
    }

    // UI에서는 현재 무게만 표시
    LOG_Item_WARNING(TEXT("[InventoryUIController::UpdateWeightUI] 현재 무게: %.1fkg"), CurrentWeight);

    // 실제 UI 위젯이 있다면:
    // WeightDisplayWidget->SetText(FText::FromString(FString::Printf(TEXT("%.1fkg"), CurrentWeight)));
}

void UInventoryUIController::ShowItemTooltip(const FBaseItemSlotData& ItemData, UWidget* TargetWidget)
{
    if (!IsLocalPlayer() || !TargetWidget)
    {
        return;
    }

    // 현재 시스템에서는 각 슬롯 위젯이 개별적으로 툴팁을 관리하므로
    // 이 함수는 로깅용으로만 사용
    LOG_Item_WARNING(TEXT("[InventoryUIController::ShowItemTooltip] 툴팁 표시: %s"),
        *ItemData.ItemRowName.ToString());
}

void UInventoryUIController::HideTooltip()
{
    if (!IsLocalPlayer())
    {
        return;
    }

    // 현재 시스템에서는 각 슬롯 위젯이 개별적으로 툴팁을 관리하므로
    // 이 함수는 로깅용으로만 사용
    LOG_Item_WARNING(TEXT("[InventoryUIController::HideTooltip] 툴팁 숨김"));
}

void UInventoryUIController::ShowBackpackUI(const TArray<FBackpackSlotData>& BackpackSlots)
{
    if (!IsLocalPlayer())
    {
        return;
    }

    ULCUIManager* UIManager = GetUIManager();
    if (UIManager)
    {
        // 인벤토리가 열려있지 않으면 자동으로 열기
        UInventoryMainWidget* InventoryWidget = GetInventoryWidget();
        if (InventoryWidget)
        {
            // 가방 인벤토리 부분만 토글 (이미 열려있으면 새로고침)
            if (!InventoryWidget->IsBackpackInventoryOpen())
            {
                InventoryWidget->ToggleBackpackInventory();
            }
            else
            {
                InventoryWidget->RefreshInventory();
            }
        }

        LOG_Item_WARNING(TEXT("[InventoryUIController::ShowBackpackUI] 가방 UI 활성화 (슬롯 수: %d)"),
            BackpackSlots.Num());
    }
}

void UInventoryUIController::HideBackpackUI()
{
    if (!IsLocalPlayer())
    {
        return;
    }

    UInventoryMainWidget* InventoryWidget = GetInventoryWidget();
    if (InventoryWidget && InventoryWidget->IsBackpackInventoryOpen())
    {
        InventoryWidget->ToggleBackpackInventory();
        LOG_Item_WARNING(TEXT("[InventoryUIController::HideBackpackUI] 가방 UI 비활성화"));
    }
}

void UInventoryUIController::Multicast_UpdateItemText_Implementation(const FText& ItemName)
{
    UpdateEquippedItemText(ItemName);
}

void UInventoryUIController::Client_RefreshUI_Implementation()
{
    RefreshInventoryUI();
}

ULCUIManager* UInventoryUIController::GetUIManager()
{
    if (CachedUIManager)
    {
        return CachedUIManager;
    }

    if (!OwnerInventory)
    {
        return nullptr;
    }

    UWorld* World = OwnerInventory->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return nullptr;
    }

    ULCGameInstanceSubsystem* GameSubsystem = GameInstance->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GameSubsystem)
    {
        return nullptr;
    }

    CachedUIManager = GameSubsystem->GetUIManager();
    return CachedUIManager;
}

UInventoryMainWidget* UInventoryUIController::GetInventoryWidget()
{
    if (CachedInventoryWidget)
    {
        return CachedInventoryWidget;
    }

    ULCUIManager* UIManager = GetUIManager();
    if (!UIManager)
    {
        return nullptr;
    }

    CachedInventoryWidget = UIManager->GetInventoryMainWidget();
    return CachedInventoryWidget;
}

bool UInventoryUIController::IsLocalPlayer() const
{
    if (!OwnerInventory)
    {
        return false;
    }

    AActor* Owner = OwnerInventory->GetOwner();
    if (!Owner)
    {
        return false;
    }

    // Pawn인 경우 로컬 컨트롤러 확인
    if (APawn* OwnerPawn = Cast<APawn>(Owner))
    {
        return OwnerPawn->IsLocallyControlled();
    }

    // PlayerController인 경우 로컬 플레이어 확인
    if (APlayerController* PC = Cast<APlayerController>(Owner))
    {
        return PC->IsLocalPlayerController();
    }

    return false;
}
