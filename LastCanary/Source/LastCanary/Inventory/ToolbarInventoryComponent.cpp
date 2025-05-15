// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "DataTable/ItemDataRow.h"
#include "Item/EquipmentItem/BackpackItem.h"
#include "LastCanary.h"

UToolbarInventoryComponent::UToolbarInventoryComponent()
{
    MaxSlots = 4;
}

void UToolbarInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    CachedOwnerCharacter = Cast<ABaseCharacter>(GetOwner());
    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::BeginPlay] CachedOwnerCharacter is null!"));
    }
}

bool UToolbarInventoryComponent::CanAddItem(AItemBase* ItemActor)
{
    if (ItemActor->bIsEquipped)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] 이미 장비한 타입의 아이템입니다."));
        // 가방에는 적용될 예정이기는 하나 어쩌면 가방을 2개 매는 상황이 나와야한다면 그냥 해당 조건은 없애야 할지도
        return false;
    }

    if (!ItemDataTable)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] ItemDataTable is null!"));
        return false;
    }

    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] CachedOwnerCharacter is null!"));
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemActor->ItemRowName, TEXT("CanAddItem"));
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] ItemData가 ItemDataTable에 없습니다."));
        return false;
    }

    if (!ItemData->AttachSocketName.IsNone())
    {
        FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(*FString::Printf(TEXT("Equipped.%s"), *ItemData->AttachSocketName.ToString()));

        if (IsEquipped(CachedOwnerCharacter, EquipTag))
        {
            LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] 해당 소겟에는 이미 아이템이 장비되어 있습니다."));
            // TODO : 이미 장착되어 있음을 알리는 UI나 메시지를 출력하면 좋을지도
            return false;
        }

        return true;
    }

    // 가방이 이미 존재할 시 체크하기 위한 코드이지만 필요없어 보임
    //const FGameplayTag BagTag = FGameplayTag::RequestGameplayTag(FName("ItemType.Equipment.Bag"));
    //if (ItemData->ItemType.MatchsTag(BagTag) && OwnerChar->BagInventoryComponent)
    //{
    //    return false;
    //}

    return true;
}

bool UToolbarInventoryComponent::TryStoreItem(AItemBase* ItemActor)
{
    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] CachedOwnerCharacter is null!"));
        return false;
    }

    if (!ItemDataTable)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] ItemDataTable is null!"));
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemActor->ItemRowName, TEXT("TryStoreItem"));
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] ItemData is null!"));
        return false;
    }

    const FGameplayTag BackpackTag = FGameplayTag::RequestGameplayTag(FName("ItemType.Equipment.Backpack"));
    if (ItemData->ItemType.MatchesTag(BackpackTag))
    {
        ABackpackItem* Backpack = Cast<ABackpackItem>(ItemActor);
        if (Backpack)
        {
            CachedOwnerCharacter->SetBackpackInventoryComponent(Backpack->GetBackpackInventoryComponent(), true);
            // TODO : MainInventoryUI를 새로고침하는 함수(정확히는 가방 인벤토리를 Visible로 바꿔야 함)
            // TODO : BackpackInventoryComponent의 소유자를 캐릭터로 변경하는 것이 안전할지도
        }
    }

    FName Socket = ItemData->AttachSocketName;
    if (Socket.IsNone())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] ItemData is null!"));
        return false;
    }

    FItemSlot NewSlot;
    NewSlot.ItemRowName = ItemActor->ItemRowName;
    NewSlot.Quantity = 1;
    NewSlot.ItemActor = ItemActor;
    ItemSlots.Add(NewSlot);

    ItemActor->AttachToComponent(CachedOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
    ItemActor->SetActorEnableCollision(false);

    RegisterEquippedItem(ItemActor, Socket);

    return true;
}

void UToolbarInventoryComponent::PostAddProcess()
{
    OnInventoryUpdated.Broadcast();
}

bool UToolbarInventoryComponent::IsSocketAvailable(ABaseCharacter* OwnerChar, FName SocketName) const
{
    if (!OwnerChar)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::IsSocketAvailable] OwnerCharacter is null!"));
        return false;
    }


    return false;
}

bool UToolbarInventoryComponent::IsEquipped(ABaseCharacter* OwnerChar, const FGameplayTag& Tag) const
{
    if (!OwnerChar)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::IsEquipped] OwnerCharacter is null!"));
        return false;
    }

    return OwnerChar->GetEquippedTags().HasTag(Tag);
}

void UToolbarInventoryComponent::RegisterEquippedItem(AItemBase* Item, FName SocketName)
{
    if (!Item)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::RegisterEquippedItem] Item is null!"));
        return;
    }

    if (SocketName.IsNone())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::RegisterEquippedItem] SocketName is none!"));
        return;
    }

    FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(*FString::Printf(TEXT("Equipped.%s"), *SocketName.ToString()));

    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::RegisterEquippedItem] OwnerCharacter is null!"));
        return;
    }

    CachedOwnerCharacter->SetEquippedTags(EquipTag, true);

}

void UToolbarInventoryComponent::UnregisterEquippedItem(FName SocketName)
{
    if (SocketName.IsNone())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnregisterEquippedItem] SocketName is none!"));
        return;
    }

    FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(*FString::Printf(TEXT("Equipped.%s"), *SocketName.ToString()));

    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::RegisterEquippedItem] OwnerCharacter is null!"));
        return;
    }

    CachedOwnerCharacter->SetEquippedTags(EquipTag, false);
}
