#include "Inventory/InventoryUtility.h"
#include "DataTable/ItemDataRow.h"
#include "Inventory/InventoryConfig.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "LastCanary.h"

bool UInventoryUtility::IsValidItem(FName ItemRowName, const UDataTable* ItemDataTable)
{
    if (ItemRowName.IsNone() || !ItemDataTable)
    {
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("IsValidItem"));
    return ItemData != nullptr;
}

bool UInventoryUtility::IsDefaultItem(FName ItemRowName, const UInventoryConfig* Config)
{
    FName DefaultName = Config ? Config->DefaultItemRowName : FName("Default");
    return ItemRowName == DefaultName;
}

bool UInventoryUtility::IsBackpackItem(const FItemDataRow* ItemData)
{
    if (!ItemData)
    {
        return false;
    }

    static const FGameplayTag BackpackTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Backpack"));
    return ItemData->ItemType.MatchesTag(BackpackTag);
}

bool UInventoryUtility::IsCollectibleItem(const FItemDataRow* ItemData)
{
    if (!ItemData)
    {
        return false;
    }

    static const FGameplayTag CollectibleTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Collectible"));
    return ItemData->ItemType.MatchesTag(CollectibleTag);
}

bool UInventoryUtility::IsWalkieTalkieItem(FName ItemRowName, const UDataTable* ItemDataTable)
{
    if (ItemRowName.IsNone() || !ItemDataTable)
    {
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("IsWalkieTalkieItem"));
    if (!ItemData)
    {
        return false;
    }

    static const FGameplayTag WalkieTalkieTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.WalkieTalkie"));
    return ItemData->ItemType.MatchesTag(WalkieTalkieTag);
}

void UInventoryUtility::SetSlotToDefault(FBaseItemSlotData& Slot, const UInventoryConfig* Config)
{
    FName DefaultName = Config ? Config->DefaultItemRowName : FName("Default");
    float DefaultDurability = Config ? Config->DefaultDurability : 100.0f;
    int32 DefaultQuantity = Config ? Config->DefaultQuantity : 1;

    Slot.ItemRowName = DefaultName;
    Slot.Quantity = DefaultQuantity;
    Slot.Durability = DefaultDurability;
    Slot.bIsBackpack = false;
    Slot.bIsValid = true;
    Slot.bIsEquipped = false;
    Slot.BackpackSlots.Empty();
    Slot.FireMode = 0;
    Slot.bWasAutoFiring = false;
}

bool UInventoryUtility::CanStackItems(const FBaseItemSlotData& Slot, FName ItemRowName, const FItemDataRow* ItemData)
{
    if (!ItemData)
    {
        return false;
    }

    return Slot.ItemRowName == ItemRowName && Slot.Quantity < ItemData->MaxStack;
}

int32 UInventoryUtility::AddToStack(FBaseItemSlotData& Slot, int32 Amount, int32 MaxStack)
{
    int32 StackSpace = MaxStack - Slot.Quantity;
    int32 Addable = FMath::Min(Amount, StackSpace);

    if (Addable > 0)
    {
        Slot.Quantity += Addable;
    }

    return Addable;
}

float UInventoryUtility::GetItemWeight(FName ItemRowName, const UDataTable* ItemDataTable)
{
    if (ItemRowName.IsNone() || !ItemDataTable)
    {
        return 1.0f;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("GetItemWeight"));
    return ItemData ? ItemData->Weight : 1.0f;
}

FItemDataRow* UInventoryUtility::GetItemDataByRowName(FName ItemRowName, const UDataTable* ItemDataTable)
{
    if (ItemRowName.IsNone() || !ItemDataTable)
    {
        return nullptr;
    }

    return ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("GetItemDataByRowName"));
}

int32 UInventoryUtility::GetItemIDFromRowName(FName ItemRowName, const UDataTable* ItemDataTable)
{
    if (ItemRowName.IsNone() || !ItemDataTable)
    {
        return -1;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("GetItemIDFromRowName"));
    return ItemData ? ItemData->ItemID : -1;
}

FName UInventoryUtility::GetItemRowNameFromID(int32 ItemID, const UDataTable* ItemDataTable)
{
    if (!ItemDataTable)
    {
        return NAME_None;
    }

    TArray<FName> RowNames = ItemDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("GetItemRowNameFromID"));
        if (ItemData && ItemData->ItemID == ItemID)
        {
            return RowName;
        }
    }

    return NAME_None;
}
