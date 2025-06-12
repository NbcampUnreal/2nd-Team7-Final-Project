#include "Item/EquipmentItem/BackpackItem.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "LastCanary.h"

ABackpackItem::ABackpackItem()
{
}

void ABackpackItem::BeginPlay()
{
    Super::BeginPlay();

    InitializeBackpackSlots(BackpackData, BackpackSlots);
}

void ABackpackItem::UseItem()
{
    if (ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner()))
    {
        if (OwnerCharacter->IsLocallyControlled())
        {
            OwnerCharacter->ToggleInventory();
        }
    }

    Super::UseItem();
}

TArray<FBackpackSlotData> ABackpackItem::GetBackpackData() const
{
    return BackpackData;
}

void ABackpackItem::SetBackpackData(const TArray<FBackpackSlotData>& InData)
{
    BackpackData = InData;
}

void ABackpackItem::InitializeBackpackSlots(TArray<FBackpackSlotData>& SlotsArray, int32 NumSlots)
{
    SlotsArray.Empty();
    for (int32 i = 0; i < NumSlots; ++i)
    {
        FBackpackSlotData DefaultSlot;
        DefaultSlot.ItemRowName = FName("Default");
        DefaultSlot.Quantity = 0;
        SlotsArray.Add(DefaultSlot);
    }
}
