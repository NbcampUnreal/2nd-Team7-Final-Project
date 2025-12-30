#include "Inventory/BackpackManager.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/InventoryUtility.h"
#include "Inventory/InventoryConfig.h"
#include "Inventory/InventoryDropSystem.h"
#include "Item/ItemSpawnerComponent.h"
#include "Item/NoteItem.h"
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
        return AddToSlot(BackpackSlot.BackpackSlots[BackpackSlotIndex], ItemRowName, Quantity, MaxStack);
    }

    // 동일한 아이템들을 모두 찾아서 스택에 추가
    for (int32 i = 0; i < BackpackSlot.BackpackSlots.Num(); ++i)
    {
        if (RemainingQuantity <= 0) break;

        FBackpackSlotData& Slot = BackpackSlot.BackpackSlots[i];

        // 동일한 아이템이고 최대 스택이 아닌 경우에만 처리
        if (Slot.ItemRowName == ItemRowName && Slot.Quantity > 0 && Slot.Quantity < MaxStack)
        {
            int32 StackSpace = MaxStack - Slot.Quantity;
            int32 Addable = FMath::Min(RemainingQuantity, StackSpace);
            Slot.Quantity += Addable;
            RemainingQuantity -= Addable;
        }
    }

    // 남은 수량이 있다면 빈 슬롯을 찾아서 새로 추가
    if (RemainingQuantity > 0)
    {
        for (int32 i = 0; i < BackpackSlot.BackpackSlots.Num(); ++i)
        {
            if (RemainingQuantity <= 0) break;

            FBackpackSlotData& Slot = BackpackSlot.BackpackSlots[i];

            // 빈 슬롯인 경우에만 새 아이템 추가
            if (OwnerInventory->IsDefaultItem(Slot.ItemRowName) || Slot.Quantity <= 0)
            {
                int32 Addable = FMath::Min(RemainingQuantity, MaxStack);
                Slot.ItemRowName = ItemRowName;
                Slot.Quantity = Addable;
                RemainingQuantity -= Addable;
            }
        }
    }

    bool bSuccess = (RemainingQuantity < OriginalQuantity);

    if (bSuccess)
    {
        OwnerInventory->UpdateWeight();
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

bool UBackpackManager::AddToSlot(FBackpackSlotData& TargetSlot, FName ItemRowName, int32 Quantity, int32 MaxStack)
{
    // 빈 슬롯인 경우
    if (OwnerInventory->IsDefaultItem(TargetSlot.ItemRowName) || TargetSlot.Quantity <= 0)
    {
        int32 Addable = FMath::Min(Quantity, MaxStack);
        TargetSlot.ItemRowName = ItemRowName;
        TargetSlot.Quantity = Addable;

        OwnerInventory->UpdateWeight();
        OwnerInventory->OnInventoryUpdated.Broadcast();
        LOG_Item_WARNING(TEXT("[AddToSlot] 빈 슬롯에 %d개 추가"), Addable);
        return (Addable == Quantity);
    }
    // 동일한 아이템인 경우 스택
    else if (TargetSlot.ItemRowName == ItemRowName)
    {
        if (TargetSlot.Quantity >= MaxStack)
        {
            LOG_Item_WARNING(TEXT("[AddToSlot] 이미 최대 스택 상태"));
            return false;
        }

        int32 StackSpace = MaxStack - TargetSlot.Quantity;
        int32 Addable = FMath::Min(Quantity, StackSpace);
        TargetSlot.Quantity += Addable;

        OwnerInventory->UpdateWeight();
        OwnerInventory->OnInventoryUpdated.Broadcast();
        LOG_Item_WARNING(TEXT("[AddToSlot] 스택에 %d개 추가 (총 %d개)"), Addable, TargetSlot.Quantity);
        return (Addable == Quantity);
    }
    // 다른 아이템인 경우
    else
    {
        LOG_Item_WARNING(TEXT("[AddToSlot] 다른 아이템으로 점유됨: %s"), *TargetSlot.ItemRowName.ToString());
        return false;
    }
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

    OwnerInventory->UpdateWeight();
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

bool UBackpackManager::UseNoteItem(int32 BackpackSlotIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("[BackpackManager] UseNoteItemFromBackpack 시작 - SlotIndex: %d"), BackpackSlotIndex);

    if (!ValidateBackpackSlot(BackpackSlotIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("[BackpackManager] 백팩 슬롯 검증 실패 - SlotIndex: %d"), BackpackSlotIndex);
        return false;
    }

    const FBaseItemSlotData& BackpackOwnerSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];
    const FBackpackSlotData& BackpackSlot = BackpackOwnerSlot.BackpackSlots[BackpackSlotIndex];

    UE_LOG(LogTemp, Warning, TEXT("[BackpackManager] 아이템 확인 - ItemRowName: %s"), *BackpackSlot.ItemRowName.ToString());

    if (!IsNoteItem(BackpackSlot.ItemRowName))
    {
        UE_LOG(LogTemp, Error, TEXT("[BackpackManager] 노트 아이템이 아님 - ItemRowName: %s"), *BackpackSlot.ItemRowName.ToString());
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("[BackpackManager] 노트 아이템 생성 및 사용 시작"));
    CreateAndUseNoteItem(BackpackSlot.ItemRowName);
    return true;
}

bool UBackpackManager::IsNoteItem(FName ItemRowName) const
{
    if (!OwnerInventory || !OwnerInventory->ItemDataTable)
    {
        return false;
    }

    const FItemDataRow* ItemData = OwnerInventory->ItemDataTable->FindRow<FItemDataRow>(
        ItemRowName, TEXT("BackpackManager::IsNoteItem"));

    return ItemData && ItemData->bIsNoteItem;
}

void UBackpackManager::CreateAndUseNoteItem(FName ItemRowName)
{
    UE_LOG(LogTemp, Warning, TEXT("[BackpackManager] CreateAndUseNoteItem 시작 - ItemRowName: %s"), *ItemRowName.ToString());

    if (!OwnerInventory || !OwnerInventory->ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[BackpackManager] ItemDataTable이 없습니다"));
        return;
    }

    const FItemDataRow* ItemData = OwnerInventory->ItemDataTable->FindRow<FItemDataRow>(
        ItemRowName, TEXT("CreateAndUseNoteItem"));
    if (!ItemData || !ItemData->ItemActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[BackpackManager] 아이템 데이터 또는 액터 클래스가 없습니다: %s"), *ItemRowName.ToString());
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[BackpackManager] 노트 아이템 액터 생성 시도"));
    ANoteItem* TempNoteItem = OwnerInventory->GetWorld()->SpawnActor<ANoteItem>(ItemData->ItemActorClass);
    if (TempNoteItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BackpackManager] 노트 아이템 액터 생성 성공"));
        TempNoteItem->ItemRowName = ItemRowName;
        TempNoteItem->SetOwner(OwnerInventory->GetOwner());
        TempNoteItem->SetInstigator(Cast<APawn>(OwnerInventory->GetOwner()));
        TempNoteItem->ApplyItemDataFromTable();
        TempNoteItem->UseItem();
        TempNoteItem->Destroy();
        UE_LOG(LogTemp, Warning, TEXT("[BackpackManager] 노트 아이템 사용 완료"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[BackpackManager] 노트 아이템 액터 생성 실패"));
    }
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
    ToolbarSlot.Quantity = 1;
    ToolbarSlot.bIsValid = true;
    ToolbarSlot.bIsEquipped = false;

    BackpackSlot.Quantity -= 1;

    // 가방 슬롯이 비었다면 Default로 설정
    if (BackpackSlot.Quantity <= 0)
    {
        BackpackSlot.ItemRowName = FName("Default");
        BackpackSlot.Quantity = 0;
    }

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

int32 UBackpackManager::MoveAndStack(int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    if (!HasBackpackEquipped())
    {
        return 0;
    }

    FBaseItemSlotData& BackpackOwnerSlot = OwnerInventory->ItemSlots[CurrentBackpackSlotIndex];

    if (!BackpackOwnerSlot.BackpackSlots.IsValidIndex(FromSlotIndex) ||
        !BackpackOwnerSlot.BackpackSlots.IsValidIndex(ToSlotIndex))
    {
        return 0;
    }

    FBackpackSlotData& FromSlot = BackpackOwnerSlot.BackpackSlots[FromSlotIndex];
    FBackpackSlotData& ToSlot = BackpackOwnerSlot.BackpackSlots[ToSlotIndex];

    // 소스 슬롯이 비어있거나 수량이 부족한 경우
    if (UInventoryUtility::IsDefaultItem(FromSlot.ItemRowName) ||
        FromSlot.Quantity <= 0 || FromSlot.Quantity < Quantity)
    {
        return 0;
    }

    // 아이템 데이터 가져오기
    const FItemDataRow* ItemData = UInventoryUtility::GetItemDataByRowName(FromSlot.ItemRowName, OwnerInventory->ItemDataTable);
    if (!ItemData)
    {
        return 0;
    }

    int32 MaxStack = ItemData->MaxStack;
    int32 ActualMoved = 0;

    // 대상 슬롯이 비어있는 경우
    if (UInventoryUtility::IsDefaultItem(ToSlot.ItemRowName) || ToSlot.Quantity <= 0)
    {
        ActualMoved = FMath::Min(Quantity, FromSlot.Quantity);
        ToSlot.ItemRowName = FromSlot.ItemRowName;
        ToSlot.Quantity = ActualMoved;
    }
    // 동일한 아이템인 경우 스택 시도
    else if (ToSlot.ItemRowName == FromSlot.ItemRowName)
    {
        int32 StackSpace = MaxStack - ToSlot.Quantity;
        int32 MaxMovable = FMath::Min(Quantity, FromSlot.Quantity);
        ActualMoved = FMath::Min(MaxMovable, StackSpace);
        ToSlot.Quantity += ActualMoved;
    }
    else
    {
        // 다른 아이템이면 스택 불가
        return 0;
    }

    // 소스 슬롯에서 수량 차감
    FromSlot.Quantity -= ActualMoved;
    if (FromSlot.Quantity <= 0)
    {
        FromSlot.ItemRowName = FName("Default");
        FromSlot.Quantity = 0;
    }

    OwnerInventory->OnInventoryUpdated.Broadcast();

    LOG_Item_WARNING(TEXT("[MoveAndStack] %d개 이동: 슬롯%d -> 슬롯%d"),
        ActualMoved, FromSlotIndex, ToSlotIndex);

    return ActualMoved;
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
    OutItemData.Durability = 0.0f;
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
