#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "DataTable/ItemDataRow.h"
#include "DataType/BaseItemSlotData.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Item/EquipmentItem/BackpackItem.h"
#include "Item/ItemBase.h"
#include "Item/ItemSpawnerComponent.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

UToolbarInventoryComponent::UToolbarInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    MaxSlots = 4;
    CurrentEquippedSlotIndex = -1;

    SetIsReplicatedByDefault(true);

    EquippedItemComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EquippedItemComponent"));

    EquippedItemComponent->SetIsReplicated(false);
    EquippedItemComponent->SetUsingAbsoluteLocation(false);
    EquippedItemComponent->SetUsingAbsoluteRotation(false);
    EquippedItemComponent->SetUsingAbsoluteScale(false);
}

void UToolbarInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!IsOwnerCharacterValid())
    {
        LOG_Item_ERROR(TEXT("[ToolbarInventoryComponent::BeginPlay] CachedOwnerCharacter가 유효하지 않습니다."));
        return;
    }

    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        false
    );

    EquippedItemComponent->AttachToComponent(
        CachedOwnerCharacter->GetMesh(),
        AttachRules,
        TEXT("Rifle")
    );
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
        UpdateWeight();
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

            UpdateWeight();
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
        LOG_Item_WARNING(TEXT("[UToolbarInventoryComponent::TrySwapItemSlots] 유효하지 않은 슬롯입니다. From: %d, To: %d"), FromIndex, ToIndex);
        return false;
    }

    int32 NewEquippedSlotIndex = CurrentEquippedSlotIndex;

    if (CurrentEquippedSlotIndex == FromIndex)
    {
        NewEquippedSlotIndex = ToIndex;
    }
    else if (CurrentEquippedSlotIndex == ToIndex)
    {
        NewEquippedSlotIndex = FromIndex;
    }

    ItemSlots.Swap(FromIndex, ToIndex);

    if (NewEquippedSlotIndex != CurrentEquippedSlotIndex)
    {
        CurrentEquippedSlotIndex = NewEquippedSlotIndex;
    }

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

void UToolbarInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UToolbarInventoryComponent, ItemSlots);
    DOREPLIFETIME(UToolbarInventoryComponent, CurrentEquippedSlotIndex);
}

void UToolbarInventoryComponent::EquipItemAtSlot(int32 SlotIndex)
{
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Error, TEXT("GetOwner() is null"));
        return;
    }

    if (!GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] Authority가 없습니다. 서버에서만 실행하세요."));
        return;
    }

    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] 유효하지 않은 슬롯 인덱스: %d"), SlotIndex);
        return;
    }

    FBaseItemSlotData* SlotData = GetItemDataAtSlot(SlotIndex);
    if (!SlotData || !SlotData->bIsValid)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] 유효하지 않은 슬롯입니다."));
        return;
    }

    if (IsDefaultItem(SlotData->ItemRowName))
    {
        LOG_Item_WARNING(TEXT("[EquipItemAtSlot] Default 아이템 장착 - 빈 손 상태로 설정"));

        // 현재 장착된 아이템 해제
        UnequipCurrentItem();

        // Default 슬롯을 장착된 상태로 설정
        ItemSlots[SlotIndex].bIsEquipped = true;
        CurrentEquippedSlotIndex = SlotIndex;

        // 캐릭터를 빈 손 상태로 설정
        if (CachedOwnerCharacter)
        {
            CachedOwnerCharacter->SetEquipped(false); // 빈 손 상태
        }

        OnInventoryUpdated.Broadcast();
        return;
    }

    ULCGameInstanceSubsystem* GISubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GISubsystem)
    {
        LOG_Item_ERROR(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] 게임 인스턴스 서브시스템이 없습니다."));
        return;
    }

    // 현재 장착된 아이템 해제
    UnequipCurrentItem();

    FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(SlotData->ItemRowName);
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] ItemData가 없습니다: %s"),
            *SlotData->ItemRowName.ToString());
        return;
    }

    static const FGameplayTag BackpackTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Backpack"));

    if (ItemData->ItemType.MatchesTag(BackpackTag))
    {
        TArray<FBaseItemSlotData> EmptyData;
        if (CachedOwnerCharacter->EquipBackpack(SlotData->ItemRowName, EmptyData, 20))
        {
            ItemSlots[SlotIndex].bIsEquipped = true;
            CurrentEquippedSlotIndex = SlotIndex;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[UToolbarInventoryComponent::EquipItemAtSlot] 가방 장착 실패"));
            return;
        }
    }
    else
    {
        if (!ItemData->ItemActorClass)
        {
            LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] ItemActorClass가 없습니다: %s"),
                *SlotData->ItemRowName.ToString());
            return;
        }

        FName TargetSocket = ItemData->AttachSocketName;
        if (TargetSocket.IsNone() || !CachedOwnerCharacter->GetMesh()->DoesSocketExist(TargetSocket))
        {
            TargetSocket = TEXT("Rifle");
        }

        EquippedItemComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

        FAttachmentTransformRules AttachRules(
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepWorld,
            false
        );

        EquippedItemComponent->AttachToComponent(
            CachedOwnerCharacter->GetMesh(),
            AttachRules,
            TargetSocket
        );

        EquippedItemComponent->SetChildActorClass(ItemData->ItemActorClass);

        AItemBase* EquippedItem = Cast<AItemBase>(EquippedItemComponent->GetChildActor());
        if (!EquippedItem)
        {
            LOG_Item_ERROR(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] ChildActor 생성 실패 또는 Cast 실패"));
            return;
        }

        EquippedItem->SetOwner(GetOwner());
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            EquippedItem->SetInstigator(OwnerPawn);
        }

        EquippedItem->ItemRowName = SlotData->ItemRowName;
        EquippedItem->Quantity = SlotData->Quantity;
        EquippedItem->Durability = SlotData->Durability;
        EquippedItem->SetActorEnableCollision(false);

        EquippedItem->ApplyItemDataFromTable();

        if (AGunBase* Gun = Cast<AGunBase>(EquippedItem))
        {
            Gun->ApplyGunDataFromDataTable();
        }

        if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem))
        {
            EquipmentItem->SetEquipped(true);
        }

        EquippedItem->ForceNetUpdate();

        ItemSlots[SlotIndex].bIsEquipped = true;
        CurrentEquippedSlotIndex = SlotIndex;
    }

    if (CachedOwnerCharacter)
    {
        CachedOwnerCharacter->SetEquipped(true);
    }

    OnInventoryUpdated.Broadcast();
}

void UToolbarInventoryComponent::UnequipCurrentItem()
{
    if (!GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] Authority가 없습니다. 서버에서만 실행하세요."));
        return;
    }

    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 장착된 아이템이 없습니다."));
        //return;
    }

    FBaseItemSlotData* SlotData = GetItemDataAtSlot(CurrentEquippedSlotIndex);
    if (!SlotData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 슬롯 데이터를 찾을 수 없습니다."));
        return;
    }

    // ⭐ 모든 아이템에 대해 bIsEquipped = false 설정
    int32 UnequipSlotIndex = CurrentEquippedSlotIndex; // 백업
    ItemSlots[UnequipSlotIndex].bIsEquipped = false;

    LOG_Item_WARNING(TEXT("[UnequipCurrentItem] 슬롯 %d의 bIsEquipped를 false로 설정: %s"),
        UnequipSlotIndex, *SlotData->ItemRowName.ToString());

    if (IsDefaultItem(SlotData->ItemRowName))
    {
        LOG_Item_WARNING(TEXT("[UnequipCurrentItem] Default 아이템 해제"));

        CurrentEquippedSlotIndex = -1;

        if (CachedOwnerCharacter)
        {
            CachedOwnerCharacter->SetEquipped(false);
        }

        OnInventoryUpdated.Broadcast();
        return;
    }

    ULCGameInstanceSubsystem* GISubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GISubsystem)
    {
        LOG_Item_ERROR(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 게임 인스턴스 서브시스템이 없습니다."));
        return;
    }

    FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(SlotData->ItemRowName);
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 아이템 데이터를 찾을 수 없습니다: %s"),
            *SlotData->ItemRowName.ToString());
        return;
    }

    static const FGameplayTag BackpackTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Backpack"));

    if (ItemData->ItemType.MatchesTag(BackpackTag))
    {
        // ⭐ 가방 아이템인 경우 실제 해제하지 않고 슬롯 상태만 변경
        LOG_Item_WARNING(TEXT("[UnequipCurrentItem] 가방 아이템 해제 - 실제 가방은 유지, 슬롯 상태만 변경"));

        CurrentEquippedSlotIndex = -1;

        // 캐릭터는 여전히 장착 상태 유지 (가방이 있으므로)
        if (CachedOwnerCharacter)
        {
            // 가방이 있으면 여전히 장착 상태로 간주
            bool bHasOtherEquipment = false;
            for (int32 i = 0; i < ItemSlots.Num(); ++i)
            {
                if (i != UnequipSlotIndex && ItemSlots[i].bIsEquipped &&
                    !IsDefaultItem(ItemSlots[i].ItemRowName))
                {
                    bHasOtherEquipment = true;
                    break;
                }
            }

            // 다른 장비가 없으면 SetEquipped(false), 있으면 유지
            if (!bHasOtherEquipment)
            {
                CachedOwnerCharacter->SetEquipped(false);
            }
        }

        // 아이템 제거
        EquippedItemComponent->DestroyChildActor();
    }
    else
    {
        // ⭐ 일반 장비 아이템 해제
        LOG_Item_WARNING(TEXT("[UnequipCurrentItem] 일반 장비 아이템 해제: %s"), *SlotData->ItemRowName.ToString());

        // 장착된 아이템 액터에서 해제 처리
        if (AItemBase* CurrentItem = Cast<AItemBase>(EquippedItemComponent->GetChildActor()))
        {
            if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(CurrentItem))
            {
                EquipmentItem->SetEquipped(false);
            }
        }

        EquippedItemComponent->DestroyChildActor();

        CurrentEquippedSlotIndex = -1;

        // 가방이 장착되어 있는지 확인
        bool bHasBackpack = CachedOwnerCharacter && CachedOwnerCharacter->HasBackpackEquipped();
        bool bHasOtherEquipment = false;

        for (int32 i = 0; i < ItemSlots.Num(); ++i)
        {
            if (i != UnequipSlotIndex && ItemSlots[i].bIsEquipped &&
                !IsDefaultItem(ItemSlots[i].ItemRowName))
            {
                bHasOtherEquipment = true;
                break;
            }
        }

        if (CachedOwnerCharacter)
        {
            // 가방이나 다른 장비가 있으면 장착 상태 유지
            CachedOwnerCharacter->SetEquipped(bHasBackpack || bHasOtherEquipment);
        }
    }

    OnInventoryUpdated.Broadcast();

    LOG_Item_WARNING(TEXT("[UnequipCurrentItem] ✅ 해제 완료: %s"), *SlotData->ItemRowName.ToString());
}

AItemBase* UToolbarInventoryComponent::GetCurrentEquippedItem() const
{
    return Cast<AItemBase>(EquippedItemComponent->GetChildActor());
}

FBaseItemSlotData* UToolbarInventoryComponent::GetItemDataAtSlot(int32 SlotIndex)
{
    if (ItemSlots.IsValidIndex(SlotIndex))
    {
        return &ItemSlots[SlotIndex];
    }
    return nullptr;
}

int32 UToolbarInventoryComponent::GetCurrentEquippedSlotIndex() const
{
    return CurrentEquippedSlotIndex;
}

bool UToolbarInventoryComponent::CanAddItem(AItemBase* ItemActor)
{
    if (!ItemActor)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] ItemActor가 null입니다."));
        return false;
    }

    if (ItemActor->bIsEquipped)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] 이미 장비한 아이템입니다."));
        return false;
    }

    if (!IsOwnerCharacterValid())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] CachedOwnerCharacter가 null입니다."));
        return false;
    }

    ULCGameInstanceSubsystem* GameSubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GameSubsystem)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] 게임 인스턴스 서브시스템이 없습니다."));
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemActor->ItemRowName, TEXT("CanAddItem"));
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] ItemData is null!"));
        return false;
    }

    for (const FBaseItemSlotData& Slot : ItemSlots)
    {
        // 빈 슬롯이거나 Default 아이템인 경우
        if (!Slot.bIsValid || IsDefaultItem(Slot.ItemRowName))
        {
            LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] ✅ 사용 가능한 슬롯 발견"));
            return true;
        }

        // 스택 가능한 슬롯 확인
        if (Slot.ItemRowName == ItemActor->ItemRowName && Slot.Quantity < ItemData->MaxStack)
        {
            LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] ✅ 스택 가능한 슬롯 발견"));
            return true;
        }
    }

    LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] 툴바에 빈 슬롯이 없습니다."));
    return false;
}

bool UToolbarInventoryComponent::TryStoreItem(AItemBase* ItemActor)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] Authority가 없습니다."));
        return false;
    }

    if (!ItemActor || !CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] ItemActor 또는 CachedOwnerCharacter가 null입니다."));
        return false;
    }

    ULCGameInstanceSubsystem* GameSubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GameSubsystem)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] 게임 인스턴스 서브시스템이 없습니다."));
        return false;
    }

    const FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(ItemActor->ItemRowName);
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] ItemData가 없습니다: %s"),
            *ItemActor->ItemRowName.ToString());
        return false;
    }

    int32 EmptySlotIndex = -1;
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (!ItemSlots[i].bIsValid || IsDefaultItem(ItemSlots[i].ItemRowName))
        {
            FBaseItemSlotData NewSlot;
            NewSlot.ItemRowName = ItemActor->ItemRowName;
            NewSlot.Quantity = ItemActor->Quantity;
            NewSlot.Durability = ItemActor->Durability;
            NewSlot.bIsValid = true;
            NewSlot.bIsEquipped = false;

            ItemSlots[i] = NewSlot;
            EmptySlotIndex = i;
            LOG_Item_WARNING(TEXT("[TryStoreItem] Default 슬롯에 아이템 저장: %s (슬롯: %d)"),
                *ItemActor->ItemRowName.ToString(), i);
            break;
        }
    }

    if (EmptySlotIndex == -1)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] 빈 슬롯이 없습니다."));
        return false;
    }

    // ⭐ 가방 아이템 특별 처리
    static const FGameplayTag BackpackTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Backpack"));
    static const FGameplayTag EquipmentTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment"));

    if (ItemData->ItemType.MatchesTag(BackpackTag))
    {
        if (ABackpackItem* BackpackItem = Cast<ABackpackItem>(ItemActor))
        {
            TArray<FBaseItemSlotData> BackpackData = BackpackItem->GetBackpackData();
            int32 BackpackMaxSlots = BackpackItem->BackpackSlots;

            TArray<FBaseItemSlotData> OldBackpackData;
            if (CachedOwnerCharacter->HasBackpackEquipped())
            {
                OldBackpackData = CachedOwnerCharacter->UnequipBackpack();

                if (OldBackpackData.Num() > 0)
                {
                    // TODO: 기존 가방을 아이템으로 드랍하는 로직 추가
                    UE_LOG(LogTemp, Warning, TEXT("[ToolbarInventoryComponent::TryStoreItem] 기존 가방에 %d개 아이템이 있었음"), OldBackpackData.Num());
                }
            }

            if (CachedOwnerCharacter->EquipBackpack(ItemActor->ItemRowName, BackpackData, BackpackMaxSlots))
            {
                ItemSlots[EmptySlotIndex].bIsEquipped = false;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[ToolbarInventoryComponent::TryStoreItem] 가방 장착 실패"));
                ItemSlots[EmptySlotIndex] = FBaseItemSlotData();
                return false;
            }
        }
    }

    OnInventoryUpdated.Broadcast();

    if (GetOwner()->HasAuthority() && ItemActor)
    {
        ItemActor->Destroy();
    }

    UpdateWeight();
    return true;
}

void UToolbarInventoryComponent::PostAddProcess()
{
    OnInventoryUpdated.Broadcast();
}

void UToolbarInventoryComponent::Server_DropEquippedItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity)
{
    Internal_DropEquippedItemAtSlot(SlotIndex, Quantity);
}

bool UToolbarInventoryComponent::DropCurrentEquippedItem()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        return Internal_DropCurrentEquippedItem();
    }
    else
    {
        Server_DropCurrentEquippedItem();
        return true;
    }
}

void UToolbarInventoryComponent::Server_DropCurrentEquippedItem_Implementation()
{
    Internal_DropCurrentEquippedItem();
}

bool UToolbarInventoryComponent::TryDropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
    if (SlotIndex == CurrentEquippedSlotIndex && CurrentEquippedSlotIndex >= 0)
    {
        if (!GetOwner() || !GetOwner()->HasAuthority())
        {
            Server_DropEquippedItemAtSlot(SlotIndex, Quantity);
            return true;
        }
        return Internal_DropEquippedItemAtSlot(SlotIndex, Quantity);
    }

    return Super::TryDropItemAtSlot(SlotIndex, Quantity);
}

bool UToolbarInventoryComponent::Internal_DropCurrentEquippedItem()
{
    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        LOG_Item_WARNING(TEXT("[Internal_DropCurrentEquippedItem] 현재 장착된 아이템이 없습니다."));
        return false;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[Internal_DropCurrentEquippedItem] Authority가 없습니다."));
        return false;
    }

    // ⭐ 간소화: Internal_DropEquippedItemAtSlot 재사용
    return Internal_DropEquippedItemAtSlot(CurrentEquippedSlotIndex, 1);
}

bool UToolbarInventoryComponent::Internal_DropEquippedItemAtSlot(int32 SlotIndex, int32 Quantity)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[Internal_DropEquippedItemAtSlot] Authority가 없습니다."));
        return false;
    }

    FBaseItemSlotData* SlotData = GetItemDataAtSlot(SlotIndex);
    if (!SlotData)
    {
        LOG_Item_WARNING(TEXT("[Internal_DropEquippedItemAtSlot] 슬롯 데이터가 없습니다."));
        return false;
    }

    // ⭐ 가방 아이템 특별 처리 (기존 Internal_DropCurrentEquippedItem 로직 활용)
    ULCGameInstanceSubsystem* GISubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (GISubsystem)
    {
        const FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(SlotData->ItemRowName);
        if (ItemData)
        {
            static const FGameplayTag BackpackTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Backpack"));

            if (ItemData->ItemType.MatchesTag(BackpackTag))
            {
                LOG_Item_WARNING(TEXT("[Internal_DropEquippedItemAtSlot] 가방 아이템 드롭 - 가방 해제"));

                // ⭐ 기존 방식: 가방 해제 후 데이터 가져오기
                TArray<FBaseItemSlotData> BackpackData = CachedOwnerCharacter->UnequipBackpack();

                FVector DropLocation = CalculateDropLocation();
                FBaseItemSlotData DropItemData = *SlotData;
                DropItemData.Quantity = FMath::Min(Quantity, SlotData->Quantity);
                DropItemData.bIsEquipped = false;

                AItemBase* DroppedItem = ItemSpawner->CreateItemFromData(DropItemData, DropLocation);
                if (DroppedItem)
                {
                    // ⭐ 기존 방식: 드롭된 가방에 데이터 설정
                    if (ABackpackItem* DroppedBackpack = Cast<ABackpackItem>(DroppedItem))
                    {
                        DroppedBackpack->SetBackpackData(BackpackData);
                        LOG_Item_WARNING(TEXT("[Internal_DropEquippedItemAtSlot] 드롭된 가방에 데이터 설정 완료: %d개"), BackpackData.Num());
                    }

                    // 슬롯에서 제거
                    SlotData->Quantity -= DropItemData.Quantity;
                    if (SlotData->Quantity <= 0)
                    {
                        SetSlotToDefault(SlotIndex);
                    }

                    // 장착 상태 해제
                    SlotData->bIsEquipped = false;
                    CurrentEquippedSlotIndex = -1;

                    // 캐릭터 상태 변경
                    if (CachedOwnerCharacter)
                    {
                        bool bHasOtherEquipment = false;
                        for (int32 i = 0; i < ItemSlots.Num(); ++i)
                        {
                            if (i != SlotIndex && ItemSlots[i].bIsEquipped &&
                                !IsDefaultItem(ItemSlots[i].ItemRowName))
                            {
                                bHasOtherEquipment = true;
                                break;
                            }
                        }

                        CachedOwnerCharacter->SetEquipped(bHasOtherEquipment);
                    }

                    UpdateWeight();
                    OnInventoryUpdated.Broadcast();

                    LOG_Item_WARNING(TEXT("[Internal_DropEquippedItemAtSlot] ✅ 가방 드롭 성공"));
                    return true;
                }
                else
                {
                    LOG_Item_WARNING(TEXT("[Internal_DropEquippedItemAtSlot] 가방 아이템 스폰 실패"));
                    return false;
                }
            }
        }
    }

    // ⭐ 일반 아이템의 경우: 장착 해제 후 베이스 클래스 드롭 로직 사용
    LOG_Item_WARNING(TEXT("[Internal_DropEquippedItemAtSlot] 일반 아이템 드롭 처리"));

    // 장착 해제 (bIsEquipped만 false로 설정, 실제 해제는 베이스 클래스에서)
    SlotData->bIsEquipped = false;
    CurrentEquippedSlotIndex = -1;

    // 장착된 액터 해제
    if (AItemBase* CurrentItem = Cast<AItemBase>(EquippedItemComponent->GetChildActor()))
    {
        if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(CurrentItem))
        {
            EquipmentItem->SetEquipped(false);
        }
    }
    EquippedItemComponent->DestroyChildActor();

    // 캐릭터 상태 변경
    if (CachedOwnerCharacter)
    {
        bool bHasBackpack = CachedOwnerCharacter->HasBackpackEquipped();
        bool bHasOtherEquipment = false;

        for (int32 i = 0; i < ItemSlots.Num(); ++i)
        {
            if (i != SlotIndex && ItemSlots[i].bIsEquipped &&
                !IsDefaultItem(ItemSlots[i].ItemRowName))
            {
                bHasOtherEquipment = true;
                break;
            }
        }

        CachedOwnerCharacter->SetEquipped(bHasBackpack || bHasOtherEquipment);
    }

    // ⭐ 베이스 클래스의 드롭 로직 실행
    return Super::Internal_TryDropItemAtSlot(SlotIndex, Quantity);
}

void UToolbarInventoryComponent::Server_DropItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity)
{
    Super::TryDropItemAtSlot(SlotIndex, Quantity);
}
