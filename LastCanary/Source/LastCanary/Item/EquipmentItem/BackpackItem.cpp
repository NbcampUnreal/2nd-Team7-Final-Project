#include "Item/EquipmentItem/BackpackItem.h"
#include "Character/BaseCharacter.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "LastCanary.h"

ABackpackItem::ABackpackItem()
{
    BackpackInventoryComponent = CreateDefaultSubobject<UBackpackInventoryComponent>(TEXT("BackpackInventoryComponent"));
}

void ABackpackItem::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        InitializeBackpack();
    }
}

void ABackpackItem::InitializeBackpack()
{
    if (BackpackInventoryComponent)
    {
        BackpackInventoryComponent->MaxSlots = BackpackSlots;
    }
}

UBackpackInventoryComponent* ABackpackItem::GetBackpackInventoryComponent() const
{
    return BackpackInventoryComponent;
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

TArray<FBaseItemSlotData> ABackpackItem::GetBackpackData() const
{
    if (BackpackInventoryComponent)
    {
        return BackpackInventoryComponent->ItemSlots;
    }
    return TArray<FBaseItemSlotData>();
}

void ABackpackItem::SetBackpackData(const TArray<FBaseItemSlotData>& NewData)
{
    if (BackpackInventoryComponent)
    {
        BackpackInventoryComponent->ItemSlots = NewData;
        BackpackInventoryComponent->OnInventoryUpdated.Broadcast();
    }
}
