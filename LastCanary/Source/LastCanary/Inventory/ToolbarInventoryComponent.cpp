// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "DataTable/ItemDataRow.h"
#include "DataType/BaseItemSlotData.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Item/EquipmentItem/BackpackItem.h"
#include "Item/ItemBase.h"
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
    EquippedBackpackComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("EquippedBackpackcomponent"));

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

    // TODO : 장비 아이템이 장착될 소켓의 위치를 정확히 판정할 필요가 있음
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,  // 위치는 소켓에 맞춤
        EAttachmentRule::SnapToTarget,  // 회전도 소켓에 맞춤
        EAttachmentRule::KeepWorld,     // 스케일은 월드 기준 유지
        false  // 용접하지 않음
    );

    EquippedBackpackComponent->AttachToComponent(
        CachedOwnerCharacter->GetMesh(),
        AttachRules,
        TEXT("BackpackSocket")
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
    if (!SlotData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] 유효하지 않은 슬롯입니다."));
        return;
    }

    ULCGameInstanceSubsystem* GameSubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GameSubsystem)
    {
        LOG_Item_ERROR(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] 게임 인스턴스 서브시스템이 없습니다."));
        return;
    }

    UnequipCurrentItem();

    FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(SlotData->ItemRowName);
    if (!ItemData || !ItemData->ItemActorClass)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] ItemData 또는 ItemActorClass가 없습니다. ItemRowName: %s"), *SlotData->ItemRowName.ToString());
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

    // 즉시 액터 확인 및 데이터 설정
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

    // 아이템 데이터 설정
    EquippedItem->ItemRowName = SlotData->ItemRowName;
    EquippedItem->Quantity = SlotData->Quantity;
    EquippedItem->Durability = SlotData->Durability;
    EquippedItem->SetActorEnableCollision(false);

    // 명시적으로 데이터 적용
    EquippedItem->ApplyItemDataFromTable();

    // 총기인 경우 추가 처리
    if (AGunBase* Gun = Cast<AGunBase>(EquippedItem))
    {
        Gun->ApplyGunDataFromDataTable();
    }

    // 장착 상태 설정
    if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem))
    {
        EquipmentItem->SetEquipped(true);
    }

    // 강제 네트워크 업데이트
    EquippedItem->ForceNetUpdate();

    // 인벤토리 상태 업데이트
    ItemSlots[SlotIndex].bIsEquipped = true;
    CurrentEquippedSlotIndex = SlotIndex;

    // 캐릭터 장착 태그 설정
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

    // 현재 장착된 슬롯이 없으면 종료
    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 장착된 아이템이 없습니다."));
        return;
    }

    // 현재 장착된 아이템 데이터 가져오기
    FBaseItemSlotData* SlotData = GetItemDataAtSlot(CurrentEquippedSlotIndex);
    if (!SlotData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 슬롯 데이터를 찾을 수 없습니다."));
        return;
    }

    // 게임 서브시스템에서 아이템 데이터 가져오기
    ULCGameInstanceSubsystem* GISubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GISubsystem)
    {
        LOG_Item_ERROR(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 게임 인스턴스 서브시스템이 없습니다."));
        return;
    }

    FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(SlotData->ItemRowName);
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 아이템 데이터를 찾을 수 없습니다: %s"), *SlotData->ItemRowName.ToString());
        return;
    }

    FName TargetSocket = ItemData->AttachSocketName;
    if (TargetSocket.IsNone())
    {
        TargetSocket = TEXT("RightHandSocket");
    }

    LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 소켓에서 아이템 제거: %s"), *TargetSocket.ToString());

    // 현재 장착된 아이템 처리
    if (AItemBase* CurrentItem = Cast<AItemBase>(EquippedItemComponent->GetChildActor()))
    {
        if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(CurrentItem))
        {
            EquipmentItem->SetEquipped(false);
        }
        else
        {
            CurrentItem->bIsEquipped = false;
        }
    }

    // 아이템 제거
    EquippedItemComponent->DestroyChildActor();

    // 슬롯 상태 업데이트
    ItemSlots[CurrentEquippedSlotIndex].bIsEquipped = false;
    CurrentEquippedSlotIndex = -1;

    // 캐릭터 상태 업데이트
    if (CachedOwnerCharacter)
    {
        CachedOwnerCharacter->SetEquipped(false);
    }

    OnInventoryUpdated.Broadcast();
}

AItemBase* UToolbarInventoryComponent::GetCurrentEquippedItem() const
{
    return Cast<AItemBase>(EquippedItemComponent->GetChildActor());
}

AItemBase* UToolbarInventoryComponent::GetCurrentEquippedBackpack() const
{
    return Cast<AItemBase>(EquippedBackpackComponent->GetChildActor());
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

    const FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(ItemActor->ItemRowName);
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::CanAddItem] ItemData가 없습니다. ItemRowName: %s"),
            *ItemActor->ItemRowName.ToString());
        return false;
    }

    for (const FBaseItemSlotData& Slot : ItemSlots)
    {
        if (!Slot.bIsValid)
        {
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

    if (!ItemActor)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] ItemActor가 null입니다."));
        return false;
    }

    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] CachedOwnerCharacter가 null입니다."));
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
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] ItemData가 없습니다. ItemRowName: %s"),
            *ItemActor->ItemRowName.ToString());
        return false;
    }


    static const FGameplayTag BackpackTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Backpack"));
    if (ItemData->ItemType.MatchesTag(BackpackTag))
    {
        if (ABackpackItem* Backpack = Cast<ABackpackItem>(ItemActor))
        {
            // 가방을 ChildActorComponent를 통해 장착
            EquippedBackpackComponent->SetChildActorClass(ItemData->ItemActorClass);

            if (ABackpackItem* EquippedBackpack = Cast<ABackpackItem>(EquippedBackpackComponent->GetChildActor()))
            {
                // 기존 가방의 데이터를 새 가방으로 복사
                if (UBackpackInventoryComponent* BackpackInventory = Backpack->GetBackpackInventoryComponent())
                {
                    // TODO : 가방을 습득할 때 새 가방으로 교체를 해야할 지 아니면 기존의 가방에 새 가방의 데이터를 가져오는 방식으로 할 지 고민
                    EquippedBackpack->CopyInventoryData(BackpackInventory);
                }

                EquippedBackpack->SetActorEnableCollision(false);
                CachedOwnerCharacter->SetBackpackInventoryComponent(EquippedBackpack->GetBackpackInventoryComponent(), true);

                LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] 가방 장착 완료: %s"),
                    *ItemData->ItemName.ToString());
            }
        }
    }

    // 빈 슬롯 찾기
    int32 EmptySlotIndex = -1;
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (!ItemSlots[i].bIsValid)
        {
            // 슬롯 데이터만 저장 (액터는 저장하지 않음)
            FBaseItemSlotData NewSlot;
            NewSlot.ItemRowName = ItemActor->ItemRowName;
            NewSlot.Quantity = ItemActor->Quantity;
            NewSlot.Durability = ItemActor->Durability;
            NewSlot.bIsValid = true;
            NewSlot.bIsEquipped = false;

            ItemSlots[i] = NewSlot;
            EmptySlotIndex = i;

            LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] 아이템 저장 성공: %s (슬롯: %d)"), 
                        *ItemActor->ItemRowName.ToString(), i);
            break;
        }
    }

    if (CachedOwnerCharacter && !CachedOwnerCharacter->IsEquipped())
    {
        // 장비 타입인 경우에만 자동 장착
        static const FGameplayTag EquipmentTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment"));
        if (ItemData->ItemType.MatchesTag(EquipmentTag) && !ItemData->ItemType.MatchesTag(BackpackTag))
        {
            EquipItemAtSlot(EmptySlotIndex);
            LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] 아이템 자동 장착: 슬롯 %d"), EmptySlotIndex);
        }
    }

    OnInventoryUpdated.Broadcast();

    if (GetOwner()->HasAuthority() && ItemActor)
    {
        ItemActor->Destroy();
    }

    return true;
}

void UToolbarInventoryComponent::PostAddProcess()
{
    OnInventoryUpdated.Broadcast();
}
