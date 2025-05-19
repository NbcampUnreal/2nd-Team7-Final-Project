// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "DataTable/ItemDataRow.h"
#include "DataType/BaseItemSlotData.h"
#include "Item/EquipmentItem/BackpackItem.h"
#include "Item/ItemBase.h"
#include "Net/UnrealNetwork.h"
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

    ItemSlots.Reserve(MaxSlots);
}

bool UToolbarInventoryComponent::TryAddItemSlot(FName ItemRowName, int32 Amount)
{
    if (Amount <= 0)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::AddItem] Amount가 0 이하"));
        return false;
    }

    if (!ItemDataTable)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::AddItem] ItemDataTable is null"));
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("AddItem"));
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::AddItem] ItemDataTable에서 아이템 탐색 실패: %s"), *ItemRowName.ToString());
        return false;
    }

    int32 RemainAmount = Amount;

    for (FBaseItemSlotData& Slot : ItemSlots)
    {
        if (Slot.ItemRowName == ItemRowName)
        {
            int32 StackSpace = ItemData->MaxStack - Slot.Quantity;
            int32 Addable = FMath::Min(RemainAmount, StackSpace);

            if (Addable > 0)
            {
                Slot.Quantity += Addable;
                RemainAmount -= Addable;

                if (RemainAmount <= 0)
                {
                    OnInventoryUpdated.Broadcast();
                    return true;
                }
            }
        }
    }

    while (RemainAmount > 0 && ItemSlots.Num() < MaxSlots)
    {
        int32 Addable = FMath::Min(RemainAmount, ItemData->MaxStack);

        FBaseItemSlotData NewSlot;
        NewSlot.ItemRowName = ItemRowName;
        NewSlot.Quantity = Addable;

        ItemSlots.Add(NewSlot);
        RemainAmount -= Addable;
    }

    if (RemainAmount == 0)
    {
        OnInventoryUpdated.Broadcast();
        return true;
    }
    else
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::AddItem] 인벤토리 공간이 부족합니다."));
        // TODO : 인벤토리가 가득 찼다는 내용의 UI 혹은 메시지를 넣어야할지도
        // 함수 실행이전에 수량 판단이 존재하므로 해당 부분까지 온다면 그냥 오류라고 봐야할 것 같다.
        return false;
    }
}

bool UToolbarInventoryComponent::TryDecreaseItem(FName ItemRowName, int32 Amount)
{
    if (Amount <= 0)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::DecreaseItem] Amount가 0 이하"));
        return false;
    }

    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (ItemSlots[i].ItemRowName == ItemRowName)
        {
            if (ItemSlots[i].Quantity < Amount)
            {
                LOG_Item_WARNING(TEXT("[InventoryComponentBase::DecreaseItem] 제거하려는 양이 재고량보다 많습니다."));
                // TODO : 재고량보다 제거하고자 하는 양이 많다는 UI를 띄우거나 메시지를 주는것이 필요할지도
                return false;
            }

            ItemSlots[i].Quantity -= Amount;

            if (ItemSlots[i].Quantity <= 0)
            {
                ItemSlots.RemoveAt(i);
            }

            return true;
        }
    }

    LOG_Item_WARNING(TEXT("[InventoryComponentBase::DecreaseItem] 존재하지 않는 아이템입니다."));
    // TODO : 실재하지 않는 아이템이라는 것을 알리는 UI나 메시지가 필요할지도
    // 예를 들면 아이템을 클릭한 상태(사용, 버리기 등의 선택지를 띄워둔 상태에서 아이템이 소모되어 사라진다면 발생할지도
    return false;
}

int32 UToolbarInventoryComponent::GetItemCount(FName ItemRowName) const
{
    int32 TotalCount = 0;

    for (const FBaseItemSlotData& Slot : ItemSlots)
    {
        if (Slot.ItemRowName == ItemRowName)
        {
            TotalCount += Slot.Quantity;
        }
    }

    return TotalCount;
}

bool UToolbarInventoryComponent::TrySwapItemSlots(int32 FromIndex, int32 ToIndex)
{
    if (!ItemSlots.IsValidIndex(FromIndex) || !ItemSlots.IsValidIndex(ToIndex))
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::SwapItemSlots] 유효하지 않은 슬롯입니다."));
        return false;
    }

    ItemSlots.Swap(FromIndex, ToIndex);
    OnInventoryUpdated.Broadcast();

    return true;
}

bool UToolbarInventoryComponent::TryRemoveItemAtSlot(int32 SlotIndex)
{
    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::RemoveItemAtSlot] 유효하지 않은 슬롯입니다."));
        // TODO : 실재하지 않는 아이템이라는 것을 알리는 UI나 메시지가 필요할지도
        // 예를 들면 아이템을 클릭한 상태(사용, 버리기 등의 선택지를 띄워둔 상태에서 아이템이 소모되어 사라진다면 발생할지도
        return false;
    }

    ItemSlots.RemoveAt(SlotIndex);
    OnInventoryUpdated.Broadcast();

    return true;
}

bool UToolbarInventoryComponent::TryAddItem(AItemBase* ItemActor)
{
    if (!CanAddItem(ItemActor))
    {
        return false;
    }

    if (!TryStoreItem(ItemActor))
    {
        return false;
    }

    PostAddProcess();

    return true;
}

void UToolbarInventoryComponent::OnRepItemSlots()
{
    OnInventoryUpdated.Broadcast();
}

void UToolbarInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UToolbarInventoryComponent, ItemSlots);
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

    static FGameplayTag EquipmentTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment"));
    if (!ItemData->ItemType.MatchesTag(EquipmentTag))
    {
        return true; // Equipment가 아닌 아이템은 무조건 추가 허용
    }

    // 2. 이미 장비 중인지 확인
    if (CachedOwnerCharacter->IsEquipped())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] 이미 장비 중인 아이템이 있습니다."));
        return false;
    }

    //if (!ItemData->AttachSocketName.IsNone())
    //{
    //    FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(*FString::Printf(TEXT("Equipped.%s"), *ItemData->AttachSocketName.ToString()));

    //    if (IsEquipped(CachedOwnerCharacter, EquipTag))
    //    {
    //        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] 해당 소겟에는 이미 아이템이 장비되어 있습니다."));
    //        // TODO : 이미 장착되어 있음을 알리는 UI나 메시지를 출력하면 좋을지도
    //        return false;
    //    }

    //    return true;
    //}

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
    if (!GetOwner() && !GetOwner()->HasAuthority())
    {
        return false;
    }

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


    // TODO : 가방 관련으로도 나중에 수정이 필요할 것으로 판단된다(아이템 액터를 제거하는 방향이 되었기에 가방이 보유한 아이템의 데이터들을 옮겨야할 듯)
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
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] Socket is null!"));
        return false;
    }

    FBaseItemSlotData NewSlot;
    NewSlot.ItemRowName = ItemActor->ItemRowName;
    NewSlot.Quantity = ItemActor->Quantity;
    NewSlot.Durability = ItemActor->Durability;

    ItemSlots.Add(NewSlot);

    ItemActor->Destroy();

    // 아이템 장착 시각적 효과 멀티캐스트
    Multicast_AttachItem(NewSlot.ItemRowName, Socket);
    return true;

    //FBaseItemSlotData NewSlot;
    //NewSlot.ItemRowName = ItemActor->ItemRowName;
    //NewSlot.Quantity = 1;
    //ItemSlots.Add(NewSlot);

    //ItemActor->AttachToComponent(CachedOwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
    //ItemActor->SetActorEnableCollision(false);

    //RegisterEquippedItem(ItemActor, Socket);

    //return true;
}

void UToolbarInventoryComponent::Multicast_AttachItem_Implementation(FName InItemRowName, FName SocketName)
{
    if (!CachedOwnerCharacter || !ItemDataTable)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::Multicast_AttachItem] CachedOwnerCharacter or ItemDataTable is null!"));
        return;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(InItemRowName, TEXT("AttachItem"));
    if (!ItemData) return;

    // 5. 새 액터 스폰 및 소켓 부착
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AItemBase* NewItem = GetWorld()->SpawnActor<AItemBase>(ItemData->ItemActorClass, FTransform::Identity, Params);

    if (NewItem)
    {
        NewItem->SetActorEnableCollision(false);
        NewItem->ItemRowName = InItemRowName;
        NewItem->ApplyItemDataFromTable();


        NewItem->AttachToComponent(CachedOwnerCharacter->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            SocketName);
    }
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

    return OwnerChar->IsEquipped();
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

    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::RegisterEquippedItem] OwnerCharacter is null!"));
        return;
    }

    CachedOwnerCharacter->SetEquipped(true);
}

void UToolbarInventoryComponent::UnregisterEquippedItem(FName SocketName)
{
    if (SocketName.IsNone())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnregisterEquippedItem] SocketName is none!"));
        return;
    }

    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::RegisterEquippedItem] OwnerCharacter is null!"));
        return;
    }

    CachedOwnerCharacter->SetEquipped(false);
}
