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
