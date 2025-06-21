#include "Inventory/BackpackManager.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/InventoryUtility.h"
#include "Inventory/InventoryConfig.h"
#include "Inventory/InventoryDropSystem.h"
#include "Item/ItemSpawnerComponent.h"
#include "DataTable/ItemDataRow.h"
#include "LastCanary.h"

UBackpackManager::UBackpackManager()
{
    OwnerInventory = nullptr;
    CurrentBackpackSlotIndex = -1;
}

void UBackpackManager::Initialize(UToolbarInventoryComponent* InOwnerInventory)
{
    OwnerInventory = InOwnerInventory;
    CurrentBackpackSlotIndex = -1;

    if (OwnerInventory)
    {
        LOG_Item_WARNING(TEXT("[BackpackManager::Initialize] 백팩 매니저 초기화 완료"));
    }
}

bool UBackpackManager::EquipBackpack(int32 ToolbarSlotIndex)
{
    if (!OwnerInventory || !OwnerInventory->ItemSlots.IsValidIndex(ToolbarSlotIndex))
    {
        return false;
    }

    FBaseItemSlotData& SlotData = OwnerInventory->ItemSlots[ToolbarSlotIndex];
    if (!SlotData.bIsBackpack)
    {
        LOG_Item_WARNING(TEXT("[BackpackManager::EquipBackpack] 가방이 아닌 아이템입니다"));
        return false;
    }

    // 기존 가방 해제
    if (CurrentBackpackSlotIndex >= 0)
    {
        UnequipBackpack(CurrentBackpackSlotIndex);
    }

    // 가방 슬롯이 부족하면 기본 20개로 채움
    if (SlotData.BackpackSlots.Num() < 20)
    {
        for (int32 i = SlotData.BackpackSlots.Num(); i < 20; ++i)
        {
            FBackpackSlotData DefaultSlot;
            DefaultSlot.ItemRowName = FName("Default");
            DefaultSlot.Quantity = 0;
            SlotData.BackpackSlots.Add(DefaultSlot);
        }
    }

    CurrentBackpackSlotIndex = ToolbarSlotIndex;

    // UI에 알림
    OnBackpackEquipped.Broadcast(SlotData.BackpackSlots);

    LOG_Item_WARNING(TEXT("[BackpackManager::EquipBackpack] 가방 장착 완료: %d개 슬롯"), SlotData.BackpackSlots.Num());
    return true;
}

bool UBackpackManager::UnequipBackpack(int32 ToolbarSlotIndex)
{
    if (CurrentBackpackSlotIndex != ToolbarSlotIndex)
    {
        return false;
    }

    CurrentBackpackSlotIndex = -1;

    // UI에 알림
    OnBackpackUnequipped.Broadcast();

    LOG_Item_WARNING(TEXT("[BackpackManager::UnequipBackpack] 가방 해제 완료"));
    return true;
}

bool UBackpackManager::HasBackpackEquipped() const
{
    return CurrentBackpackSlotIndex >= 0 &&
        OwnerInventory &&
        OwnerInventory->ItemSlots.IsValidIndex(CurrentBackpackSlotIndex) &&
        OwnerInventory->ItemSlots[CurrentBackpackSlotIndex].bIsBackpack;
}

TArray<FBackpackSlotData> UBackpackManager::GetCurrentBackpackSlots() const
{
    if (HasBackpackEquipped())
    {
        return OwnerInventory->ItemSlots[CurrentBackpackSlotIndex].BackpackSlots;
    }
    return TArray<FBackpackSlotData>();
}

bool UBackpackManager::AddItemToBackpack(FName ItemRowName, int32 Quantity, int32 BackpackSlotIndex)
{
    int32 TargetBackpackSlotIndex = -1;

    if (HasBackpackEquipped())
    {
        // 장착된 가방 사용
        TargetBackpackSlotIndex = CurrentBackpackSlotIndex;
        LOG_Item_WARNING(TEXT("[AddItemToBackpack] 장착된 가방 사용: 슬롯 %d"), TargetBackpackSlotIndex);
    }
    else
    {
        // 장착되지 않은 가방 중 첫 번째 가방 찾기
        for (int32 i = 0; i < OwnerInventory->ItemSlots.Num(); ++i)
        {
            if (OwnerInventory->ItemSlots[i].bIsBackpack)
            {
                TargetBackpackSlotIndex = i;
                LOG_Item_WARNING(TEXT("[AddItemToBackpack] 장착되지 않은 가방 사용: 슬롯 %d"), TargetBackpackSlotIndex);
                break;
            }
        }
    }

    // 가방이 없으면 실패
    if (TargetBackpackSlotIndex < 0 || !OwnerInventory->ItemSlots.IsValidIndex(TargetBackpackSlotIndex))
    {
        LOG_Item_WARNING(TEXT("[AddItemToBackpack] 사용할 수 있는 가방이 없습니다"));
        return false;
    }

    if (!CanAddToBackpack(ItemRowName))
    {
        LOG_Item_WARNING(TEXT("[AddItemToBackpack] 가방에 넣을 수 없는 아이템: %s"), *ItemRowName.ToString());
        return false;
    }

    FBaseItemSlotData& BackpackSlot = OwnerInventory->ItemSlots[TargetBackpackSlotIndex];

    // 가방이 맞는지 확인
    if (!BackpackSlot.bIsBackpack)
    {
        LOG_Item_WARNING(TEXT("[AddItemToBackpack] 슬롯 %d는 가방이 아닙니다"), TargetBackpackSlotIndex);
        return false;
    }

    const FItemDataRow* ItemData = UInventoryUtility::GetItemDataByRowName(ItemRowName, OwnerInventory->ItemDataTable);
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[AddItemToBackpack] 아이템 데이터를 찾을 수 없음: %s"), *ItemRowName.ToString());
        return false;
    }

    int32 MaxStack = ItemData->MaxStack;
    int32 RemainingQuantity = Quantity;
    int32 OriginalQuantity = RemainingQuantity;

    // 특정 슬롯 지정된 경우
    if (BackpackSlotIndex >= 0 && BackpackSlot.BackpackSlots.IsValidIndex(BackpackSlotIndex))
    {
        FBackpackSlotData& TargetSlot = BackpackSlot.BackpackSlots[BackpackSlotIndex];

        if (OwnerInventory->IsDefaultItem(TargetSlot.ItemRowName) || TargetSlot.Quantity <= 0)
        {
            int32 Addable = FMath::Min(RemainingQuantity, MaxStack);
            TargetSlot.ItemRowName = ItemRowName;
            TargetSlot.Quantity = Addable;
            RemainingQuantity -= Addable;
        }
        else if (TargetSlot.ItemRowName == ItemRowName)
        {
            int32 StackSpace = MaxStack - TargetSlot.Quantity;
            int32 Addable = FMath::Min(RemainingQuantity, StackSpace);
            TargetSlot.Quantity += Addable;
            RemainingQuantity -= Addable;
        }
        else
        {
            LOG_Item_WARNING(TEXT("[AddItemToBackpack] 슬롯이 다른 아이템으로 점유됨: %s"), *TargetSlot.ItemRowName.ToString());
        }
    }
    else
    {
        // 자동으로 빈 슬롯 찾기
        for (int32 i = 0; i < BackpackSlot.BackpackSlots.Num(); ++i)
        {
            if (RemainingQuantity <= 0) break;

            FBackpackSlotData& Slot = BackpackSlot.BackpackSlots[i];

            if (OwnerInventory->IsDefaultItem(Slot.ItemRowName) || Slot.Quantity <= 0)
            {
                // 빈 슬롯에 새 아이템 추가
                int32 Addable = FMath::Min(RemainingQuantity, MaxStack);
                Slot.ItemRowName = ItemRowName;
                Slot.Quantity = Addable;
                RemainingQuantity -= Addable;
            }
            else if (Slot.ItemRowName == ItemRowName)
            {
                // 같은 아이템 스택에 추가
                int32 StackSpace = MaxStack - Slot.Quantity;
                if (StackSpace > 0)
                {
                    int32 Addable = FMath::Min(RemainingQuantity, StackSpace);
                    Slot.Quantity += Addable;
                    RemainingQuantity -= Addable;
                }
            }
        }
    }

    bool bSuccess = (RemainingQuantity < OriginalQuantity);

    if (bSuccess)
    {
        OwnerInventory->OnInventoryUpdated.Broadcast();
        LOG_Item_WARNING(TEXT("[AddItemToBackpack] 성공: %d/%d개 추가됨"),
            OriginalQuantity - RemainingQuantity, OriginalQuantity);
    }
    else
    {
        LOG_Item_WARNING(TEXT("[AddItemToBackpack] 실패: 추가할 공간이 없음"));
    }

    return bSuccess;
}

bool UBackpackManager::RemoveItemFromBackpack(int32 BackpackSlotIndex, int32 Quantity)
{
    if (!HasBackpackEquipped())
    {
        return false;
    }

    FBaseItemSlotData& BackpackSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];
    if (!BackpackSlot.BackpackSlots.IsValidIndex(BackpackSlotIndex))
    {
        return false;
    }

    FBackpackSlotData& TargetSlot = BackpackSlot.BackpackSlots[BackpackSlotIndex];
    if (TargetSlot.Quantity < Quantity)
    {
        return false;
    }

    TargetSlot.Quantity -= Quantity;
    if (TargetSlot.Quantity <= 0)
    {
        TargetSlot.ItemRowName = FName("Default");
        TargetSlot.Quantity = 0;
    }

    OwnerInventory->OnInventoryUpdated.Broadcast();
    return true;
}

bool UBackpackManager::UpdateBackpackSlots(const TArray<FBackpackSlotData>& NewSlots)
{
    if (!HasBackpackEquipped())
    {
        return false;
    }

    OwnerInventory->ItemSlots[CurrentBackpackSlotIndex].BackpackSlots = NewSlots;
    OwnerInventory->OnInventoryUpdated.Broadcast();
    return true;
}

bool UBackpackManager::MoveToolbarItemToBackpack(int32 ToolbarIndex, int32 BackpackIndex)
{
    if (!OwnerInventory || !OwnerInventory->ItemSlots.IsValidIndex(ToolbarIndex) || !HasBackpackEquipped())
    {
        return false;
    }

    FBaseItemSlotData& ToolbarSlot = OwnerInventory->ItemSlots[ToolbarIndex];

    // Default 아이템은 이동 불가
    if (UInventoryUtility::IsDefaultItem(ToolbarSlot.ItemRowName))
    {
        return false;
    }

    // 수집품만 가방에 넣을 수 있는지 확인
    if (!CanAddToBackpack(ToolbarSlot.ItemRowName))
    {
        return false;
    }

    FBaseItemSlotData& BackpackOwnerSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];
    if (!BackpackOwnerSlot.BackpackSlots.IsValidIndex(BackpackIndex))
    {
        return false;
    }

    FBackpackSlotData& BackpackSlot = BackpackOwnerSlot.BackpackSlots[BackpackIndex];

    // 대상 슬롯이 비어있어야 함
    if (!UInventoryUtility::IsDefaultItem(BackpackSlot.ItemRowName) && BackpackSlot.Quantity > 0)
    {
        return false;
    }

    // 이동 실행
    BackpackSlot.ItemRowName = ToolbarSlot.ItemRowName;
    BackpackSlot.Quantity = ToolbarSlot.Quantity;

    // 툴바 슬롯 초기화
    UInventoryUtility::SetSlotToDefault(ToolbarSlot, OwnerInventory->GetInventoryConfig());

    OwnerInventory->OnInventoryUpdated.Broadcast();
    return true;
}

bool UBackpackManager::MoveBackpackItemToToolbar(int32 BackpackIndex, int32 ToolbarIndex)
{
    if (!OwnerInventory || !OwnerInventory->ItemSlots.IsValidIndex(ToolbarIndex) || !HasBackpackEquipped())
    {
        return false;
    }

    FBaseItemSlotData& ToolbarSlot = OwnerInventory->ItemSlots[ToolbarIndex];
    FBaseItemSlotData& BackpackOwnerSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];

    if (!BackpackOwnerSlot.BackpackSlots.IsValidIndex(BackpackIndex))
    {
        return false;
    }

    FBackpackSlotData& BackpackSlot = BackpackOwnerSlot.BackpackSlots[BackpackIndex];

    // 소스가 비어있으면 이동 불가
    if (UInventoryUtility::IsDefaultItem(BackpackSlot.ItemRowName) || BackpackSlot.Quantity <= 0)
    {
        return false;
    }

    // 대상이 비어있어야 함
    if (!UInventoryUtility::IsDefaultItem(ToolbarSlot.ItemRowName))
    {
        return false;
    }

    // 이동 실행
    ToolbarSlot.ItemRowName = BackpackSlot.ItemRowName;
    ToolbarSlot.Quantity = BackpackSlot.Quantity;
    ToolbarSlot.bIsValid = true;
    ToolbarSlot.bIsEquipped = false;

    // 가방 슬롯 초기화
    BackpackSlot.ItemRowName = FName("Default");
    BackpackSlot.Quantity = 0;

    OwnerInventory->OnInventoryUpdated.Broadcast();
    return true;
}

bool UBackpackManager::SwapBackpackSlots(int32 FromIndex, int32 ToIndex)
{
    if (!HasBackpackEquipped())
    {
        return false;
    }

    FBaseItemSlotData& BackpackOwnerSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];

    if (!BackpackOwnerSlot.BackpackSlots.IsValidIndex(FromIndex) ||
        !BackpackOwnerSlot.BackpackSlots.IsValidIndex(ToIndex))
    {
        return false;
    }

    BackpackOwnerSlot.BackpackSlots.Swap(FromIndex, ToIndex);
    OwnerInventory->OnInventoryUpdated.Broadcast();
    return true;
}

bool UBackpackManager::DropItemFromBackpack(int32 BackpackSlotIndex, int32 Quantity)
{
    if (!OwnerInventory || !HasBackpackEquipped())
    {
        return false;
    }

    FBaseItemSlotData ItemData;
    if (!ConvertBackpackSlotToItemData(BackpackSlotIndex, 1, ItemData))
    {
        return false;
    }

    UItemSpawnerComponent* ItemSpawner = OwnerInventory->GetItemSpawner();
    if (!ItemSpawner)
    {
        return false;
    }

    FVector DropLocation = UInventoryDropSystem::CalculateDropLocation(OwnerInventory->GetOwner(), OwnerInventory->GetInventoryConfig());

    for (int32 i = 0; i < Quantity; ++i)
    {
        ItemData.Quantity = 1;

        AItemBase* DroppedItem = ItemSpawner->CreateItemFromData(ItemData, DropLocation);
        if (!DroppedItem)
        {
            LOG_Item_WARNING(TEXT("[DropItemFromBackpack] 아이템 생성 실패: %s (%d/%d)"), *ItemData.ItemRowName.ToString(), i + 1, Quantity);
        }
    }

    FBaseItemSlotData& BackpackOwnerSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];
    FBackpackSlotData& BackpackSlot = BackpackOwnerSlot.BackpackSlots[BackpackSlotIndex];

    BackpackSlot.Quantity -= Quantity;
    if (BackpackSlot.Quantity <= 0)
    {
        BackpackSlot.ItemRowName = FName("Default");
        BackpackSlot.Quantity = 0;
    }

    OwnerInventory->UpdateWeight();
    OwnerInventory->UpdateWalkieTalkieChannelStatus();
    OwnerInventory->OnInventoryUpdated.Broadcast();

    LOG_Item_WARNING(TEXT("[DropItemFromBackpack] 백팩 드롭 완료: %s x%d"), *ItemData.ItemRowName.ToString(), Quantity);

    return true;
}

bool UBackpackManager::ConvertBackpackSlotToItemData(int32 BackpackSlotIndex, int32 Quantity, FBaseItemSlotData& OutItemData)
{
    if (!ValidateBackpackSlot(BackpackSlotIndex))
    {
        return false;
    }

    const FBaseItemSlotData& BackpackOwnerSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];
    const FBackpackSlotData& BackpackSlot = BackpackOwnerSlot.BackpackSlots[BackpackSlotIndex];

    // Default 아이템 체크
    if (UInventoryUtility::IsDefaultItem(BackpackSlot.ItemRowName, OwnerInventory->GetInventoryConfig()))
    {
        return false;
    }

    // 수량 검증
    if (BackpackSlot.Quantity <= 0 || Quantity > BackpackSlot.Quantity || Quantity <= 0)
    {
        return false;
    }

    // 단순히 데이터 변환만
    OutItemData.ItemRowName = BackpackSlot.ItemRowName;
    OutItemData.Quantity = Quantity;
    OutItemData.Durability = 100.0f;
    OutItemData.bIsValid = true;
    OutItemData.bIsEquipped = false;
    OutItemData.bIsBackpack = false;

    return true;
}

int32 UBackpackManager::FindEmptyBackpackSlot(const TArray<FBackpackSlotData>& BackpackSlots) const
{
    for (int32 i = 0; i < BackpackSlots.Num(); ++i)
    {
        if (UInventoryUtility::IsDefaultItem(BackpackSlots[i].ItemRowName) || BackpackSlots[i].Quantity <= 0)
        {
            return i;
        }
    }
    return -1;
}

bool UBackpackManager::ValidateBackpackSlot(int32 BackpackSlotIndex) const
{
    if (!HasBackpackEquipped())
    {
        return false;
    }

    const FBaseItemSlotData& BackpackSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];
    return BackpackSlot.BackpackSlots.IsValidIndex(BackpackSlotIndex);
}

bool UBackpackManager::CanAddToBackpack(FName ItemRowName) const
{
    if (!OwnerInventory || !OwnerInventory->ItemDataTable)
    {
        return false;
    }

    const FItemDataRow* ItemData = UInventoryUtility::GetItemDataByRowName(ItemRowName, OwnerInventory->ItemDataTable);
    return UInventoryUtility::IsCollectibleItem(ItemData);
}
