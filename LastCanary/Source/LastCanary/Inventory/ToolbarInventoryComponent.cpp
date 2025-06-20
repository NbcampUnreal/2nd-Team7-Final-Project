#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/InventoryUtility.h"
#include "Inventory/InventoryDropSystem.h"
#include "Inventory/InventoryUIController.h"
#include "Inventory/InventoryConfig.h"
#include "Character/BaseCharacter.h"
#include "Character/BasePlayerState.h"
#include "DataTable/ItemDataRow.h"
#include "DataType/BaseItemSlotData.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Item/EquipmentItem/BackpackItem.h"
#include "Item/ItemBase.h"
#include "Item/ItemSpawnerComponent.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Item/EquipmentItem/GunBase.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/Manager/LCUIManager.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
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

    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
    EquippedItemComponent->AttachToComponent(CachedOwnerCharacter->GetMesh(), AttachRules, TEXT("Rifle"));

    InitializeManagers();
}

void UToolbarInventoryComponent::InitializeManagers()
{
    // 백팩 매니저 초기화
    BackpackManager = NewObject<UBackpackManager>(this);
    if (BackpackManager)
    {
        BackpackManager->Initialize(this);

        // 델리게이트 연결
        BackpackManager->OnBackpackEquipped.AddDynamic(this, &UToolbarInventoryComponent::OnBackpackEquippedHandler);
        BackpackManager->OnBackpackUnequipped.AddDynamic(this, &UToolbarInventoryComponent::OnBackpackUnequippedHandler);
    }

    // UI 컨트롤러 초기화
    UIController = NewObject<UInventoryUIController>(this);
    if (UIController)
    {
        UIController->Initialize(this);
    }

    LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent] 모든 매니저 초기화 완료"));
}

bool UToolbarInventoryComponent::TryAddItemSlot(FName ItemRowName, int32 Amount)
{
    if (Amount <= 0)
    {
        LOG_Item_WARNING(TEXT("[TryAddItemSlot] Amount가 0 이하"));
        return false;
    }

    if (!ItemDataTable)
    {
        LOG_Item_WARNING(TEXT("[TryAddItemSlot] ItemDataTable is null"));
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("AddItem"));
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[TryAddItemSlot] ItemDataTable에서 아이템 탐색 실패: %s"), *ItemRowName.ToString());
        return false;
    }

    int32 RemainAmount = Amount;

    for (FBaseItemSlotData& Slot : ItemSlots)
    {
        if (UInventoryUtility::CanStackItems(Slot, ItemRowName, ItemData))
        {
            int32 Addable = UInventoryUtility::AddToStack(Slot, RemainAmount, ItemData->MaxStack);
            RemainAmount -= Addable;

            if (RemainAmount <= 0)
            {
                UpdateWeight();
                UpdateWalkieTalkieChannelStatus();
                OnInventoryUpdated.Broadcast();
                return true;
            }
        }
    }

    while (RemainAmount > 0 && ItemSlots.Num() < MaxSlots)
    {
        int32 Addable = FMath::Min(RemainAmount, ItemData->MaxStack);

        FBaseItemSlotData NewSlot;
        NewSlot.ItemRowName = ItemRowName;
        NewSlot.Quantity = Addable;
        NewSlot.Durability = 100.0f;
        NewSlot.bIsValid = true;
        NewSlot.bIsEquipped = false;

        ItemSlots.Add(NewSlot);
        RemainAmount -= Addable;
    }

    if (RemainAmount == 0)
    {
        UpdateWeight();
        UpdateWalkieTalkieChannelStatus();
        OnInventoryUpdated.Broadcast();
        return true;
    }
    else
    {
        LOG_Item_WARNING(TEXT("[TryAddItemSlot] 인벤토리 공간이 부족합니다."));
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
        LOG_Item_WARNING(TEXT("[TryRemoveItemAtSlot] 유효하지 않은 슬롯입니다."));
        return false;
    }

    UInventoryUtility::SetSlotToDefault(ItemSlots[SlotIndex], GetInventoryConfig());

    SyncInventoryToPlayerState();
    UpdateWalkieTalkieChannelStatus();
    OnInventoryUpdated.Broadcast();

    return true;
}

bool UToolbarInventoryComponent::TryAddItem(AItemBase* ItemActor)
{
    if (!ItemActor || !IsOwnerCharacterValid())
    {
        return false;
    }

    ULCGameInstanceSubsystem* GameSubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GameSubsystem)
    {
        return false;
    }

    const FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(ItemActor->ItemRowName);
    if (!ItemData)
    {
        return false;
    }

    // 수집 아이템(Collectible)일 때 가방이 있다면 가방 슬롯에 추가
    if (IsCollectibleItem(ItemData))
    {
        if (BackpackManager && BackpackManager->AddItemToBackpack(ItemActor->ItemRowName, ItemActor->Quantity, -1))
        {
            if (GetOwner()->HasAuthority())
                ItemActor->Destroy();
            PostAddProcess();
            return true;
        }
    }

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
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] GetOwner() is null"));
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

    // 기존의 장착된 아이템 해제
    UnequipCurrentItem();

    // 기존 슬롯이 Default 상태일 때
    if (IsDefaultItem(SlotData->ItemRowName))
    {
        ItemSlots[SlotIndex].bIsEquipped = true;
        CurrentEquippedSlotIndex = SlotIndex;

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
        LOG_Item_ERROR(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] 게임 인스턴스 서브시스템이 없습니다."));
        return;
    }

    FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(SlotData->ItemRowName);
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::EquipItemAtSlot] ItemData가 없습니다: %s"), *SlotData->ItemRowName.ToString());
        return;
    }

    if (IsBackpackItem(ItemData))
    {
        HandleBackpackEquip(SlotIndex);
    }
    else
    {
        // 일반 장비 처리
        FName TargetSocket = ItemData->AttachSocketName.IsNone() ? TEXT("Rifle") : ItemData->AttachSocketName;
        if (!CachedOwnerCharacter->GetMesh()->DoesSocketExist(TargetSocket))
        {
            TargetSocket = TEXT("Rifle");
        }
        SetupEquippedItem(EquippedItemComponent, CachedOwnerCharacter->GetMesh(), TargetSocket, ItemData, SlotData);
    }

    // 공통 마무리
    ItemSlots[SlotIndex].bIsEquipped = true;
    CurrentEquippedSlotIndex = SlotIndex;
    if (CachedOwnerCharacter) CachedOwnerCharacter->SetEquipped(true);

    // UI 처리
    if (GetOwner()->HasAuthority())
    {
        if (ULCUIManager* UIManager = GISubsystem->GetUIManager())
        {
            if (UInventoryMainWidget* InventoryWidget = UIManager->GetInventoryMainWidget())
            {
                FText DisplayText = FText::FromString(ItemData->ItemName.ToString());
                MulticastUpdateItemText(DisplayText);
            }
        }
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
        return;
    }

    SyncEquippedItemDurabilityToSlot();
    SyncGunStateToSlot();

    FBaseItemSlotData* SlotData = GetItemDataAtSlot(CurrentEquippedSlotIndex);
    if (!SlotData)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::UnequipCurrentItem] 슬롯 데이터를 찾을 수 없습니다."));
        CurrentEquippedSlotIndex = -1;
        return;
    }

    // 슬롯 상태 업데이트
    int32 UnequipSlotIndex = CurrentEquippedSlotIndex;
    ItemSlots[UnequipSlotIndex].bIsEquipped = false;

    // Default 아이템 처리
    if (IsDefaultItem(SlotData->ItemRowName))
    {
        CurrentEquippedSlotIndex = -1;
        if (CachedOwnerCharacter) CachedOwnerCharacter->SetEquipped(false);
        OnInventoryUpdated.Broadcast();
        return;
    }

    // 가방 전용 전처리
    if (SlotData->bIsBackpack)
    {
        HandleBackpackUnequip(UnequipSlotIndex);
    }
    else
    {
        // ✅ 일반 장비 해제
        if (AItemBase* CurrentItem = Cast<AItemBase>(EquippedItemComponent->GetChildActor()))
        {
            if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(CurrentItem))
            {
                EquipmentItem->SetEquipped(false);
            }
        }
        EquippedItemComponent->DestroyChildActor();
    }

    CurrentEquippedSlotIndex = -1;
    bool bHasOtherEquipment = HasOtherEquippedItems();
    if (CachedOwnerCharacter) CachedOwnerCharacter->SetEquipped(bHasOtherEquipment);

    OnInventoryUpdated.Broadcast();
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

void UToolbarInventoryComponent::SyncEquippedItemDurabilityToSlot()
{
    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        return;
    }

    AItemBase* EquippedItem = GetCurrentEquippedItem();
    if (!EquippedItem)
    {
        return;
    }

    bool bShouldSync = false;

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        bShouldSync = true; // 서버에서는 항상 동기화
    }
    else if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        bShouldSync = OwnerPawn->IsLocallyControlled(); // 클라이언트에서는 로컬만
    }

    if (bShouldSync)
    {
        ItemSlots[CurrentEquippedSlotIndex].Durability = EquippedItem->Durability;
        // 총기인 경우 추가 상태 동기화
        if (AGunBase* Gun = Cast<AGunBase>(EquippedItem))
        {
            FBaseItemSlotData& SlotData = ItemSlots[CurrentEquippedSlotIndex];
            SlotData.FireMode = static_cast<int32>(Gun->CurrentFireMode);
            SlotData.bWasAutoFiring = Gun->bIsAutoFiring;
        }

        OnInventoryUpdated.Broadcast();
    }
}

void UToolbarInventoryComponent::SyncGunStateToSlot()
{
    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        return;
    }

    AItemBase* EquippedItem = GetCurrentEquippedItem();
    AGunBase* Gun = Cast<AGunBase>(EquippedItem);
    if (!Gun)
    {
        return;
    }

    bool bShouldSync = false;
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        bShouldSync = true; // 서버에서는 항상 동기화
    }
    else if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        bShouldSync = OwnerPawn->IsLocallyControlled(); // 클라이언트에서는 로컬만
    }

    if (bShouldSync)
    {
        FBaseItemSlotData& SlotData = ItemSlots[CurrentEquippedSlotIndex];

        // 내구도 동기화
        SlotData.Durability = Gun->Durability;
        // 총기 상태 동기화 
        SlotData.FireMode = static_cast<int32>(Gun->CurrentFireMode);
        SlotData.bWasAutoFiring = Gun->bIsAutoFiring;

        OnInventoryUpdated.Broadcast();
    }
}

void UToolbarInventoryComponent::RestoreGunStateFromSlot(AGunBase* Gun, const FBaseItemSlotData& SlotData)
{
    if (!Gun)
    {
        return;
    }

    // 총기 상태 복원
    Gun->CurrentFireMode = static_cast<EFireMode>(SlotData.FireMode);
}

int32 UToolbarInventoryComponent::GetCurrentEquippedSlotIndex() const
{
    return CurrentEquippedSlotIndex;
}

void UToolbarInventoryComponent::SetupEquippedItem(UChildActorComponent* ItemComponent, USkeletalMeshComponent* TargetMesh, FName SocketName, FItemDataRow* ItemData, FBaseItemSlotData* SlotData)
{
    if (!ItemComponent || !TargetMesh || !ItemData || !SlotData)
    {
        return;
    }

    // 기존 아이템 해제
    ItemComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

    // 새 소켓에 연결
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        false
    );

    ItemComponent->AttachToComponent(TargetMesh, AttachRules, SocketName);
    ItemComponent->SetChildActorClass(ItemData->ItemActorClass);

    // 아이템 설정
    AItemBase* EquippedItem = Cast<AItemBase>(ItemComponent->GetChildActor());
    if (!EquippedItem)
    {
        LOG_Item_ERROR(TEXT("[SetupEquippedItem] ChildActor 생성 실패"));
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
    // 상위 콜리전 설정이라서 개별 설정을 하더라도 해당 설정을 우선함
    // EquippedItem->SetActorEnableCollision(false);

    if (UStaticMeshComponent* StaticMesh = EquippedItem->FindComponentByClass<UStaticMeshComponent>())
    {
        StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (USkeletalMeshComponent* SkeletalMesh = EquippedItem->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    EquippedItem->ApplyItemDataFromTable();


    if (AGunBase* Gun = Cast<AGunBase>(EquippedItem))
    {
        Gun->ApplyGunDataFromDataTable();
        RestoreGunStateFromSlot(Gun, *SlotData);
    }

    if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem))
    {
        EquipmentItem->SetEquipped(true);
    }

    if (ABackpackItem* BackpackItem = Cast<ABackpackItem>(EquippedItem))
    {
        BackpackItem->bMeshVisible = false; // 자동으로 복제됨
    }

    EquippedItem->ForceNetUpdate();
}

bool UToolbarInventoryComponent::CanAddItem(AItemBase* ItemActor)
{
    if (!ItemActor)
    {
        LOG_Item_WARNING(TEXT("[CanAddItem] ItemActor가 null입니다."));
        return false;
    }

    if (ItemActor->bIsEquipped)
    {
        LOG_Item_WARNING(TEXT("[CanAddItem] 이미 장비한 아이템입니다."));
        return false;
    }

    if (!IsOwnerCharacterValid())
    {
        LOG_Item_WARNING(TEXT("[CanAddItem] CachedOwnerCharacter가 null입니다."));
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemActor->ItemRowName, TEXT("CanAddItem"));
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[CanAddItem] ItemData is null!"));
        return false;
    }

    for (const FBaseItemSlotData& Slot : ItemSlots)
    {
        // 빈 슬롯이거나 Default 아이템인 경우
        if (!Slot.bIsValid || UInventoryUtility::IsDefaultItem(Slot.ItemRowName, GetInventoryConfig()))
        {
            return true;
        }

        if (UInventoryUtility::CanStackItems(Slot, ItemActor->ItemRowName, ItemData))
        {
            return true;
        }
    }

    LOG_Item_WARNING(TEXT("[CanAddItem] 툴바에 빈 슬롯이 없습니다."));
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
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] ItemData가 없습니다: %s"), *ItemActor->ItemRowName.ToString());
        return false;
    }

    // 빈 슬롯 찾기
    int32 EmptySlotIndex = -1;
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (!ItemSlots[i].bIsValid || IsDefaultItem(ItemSlots[i].ItemRowName))
        {
            EmptySlotIndex = i;
            break;
        }
    }

    if (EmptySlotIndex == -1)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] 빈 슬롯 없음"));
        return false;
    }

    // 슬롯 데이터 생성
    FBaseItemSlotData NewSlot;
    NewSlot.ItemRowName = ItemActor->ItemRowName;
    NewSlot.Quantity = ItemActor->Quantity;
    NewSlot.Durability = ItemActor->Durability;
    NewSlot.bIsValid = true;
    NewSlot.bIsEquipped = false;

    // 가방 전용 처리
    if (IsBackpackItem(ItemData))
    {
        NewSlot.bIsBackpack = true;
        NewSlot.BackpackSlots.Empty();
        // 데이터가 있다면 복사
        if (ABackpackItem* BackpackItem = Cast<ABackpackItem>(ItemActor))
        {
            TArray<FBackpackSlotData> BackpackData = BackpackItem->GetBackpackData();
            NewSlot.BackpackSlots = BackpackData;
        }
        // 부족하면 Default로 채움
        if (NewSlot.BackpackSlots.Num() < 20)
        {
            for (int32 i = NewSlot.BackpackSlots.Num(); i < 20; ++i)
            {
                FBackpackSlotData DefaultSlot;
                DefaultSlot.ItemRowName = FName("Default");
                DefaultSlot.Quantity = 0;
                NewSlot.BackpackSlots.Add(DefaultSlot);
            }
        }
    }

    // 총기 상태 저장
    if (AGunBase* Gun = Cast<AGunBase>(ItemActor))
    {
        NewSlot.FireMode = static_cast<int32>(Gun->CurrentFireMode);
        NewSlot.bWasAutoFiring = Gun->bIsAutoFiring;
    }

    ItemSlots[EmptySlotIndex] = NewSlot;

    // 정리
    SyncInventoryToPlayerState();
    OnInventoryUpdated.Broadcast();
    if (GetOwner()->HasAuthority() && ItemActor)
    {
        ItemActor->Destroy();
    }
    UpdateWeight();
    UpdateWalkieTalkieChannelStatus();

    LOG_Item_WARNING(TEXT("[ToolbarInventoryComponent::TryStoreItem] 저장 성공: %s (슬롯: %d)"), *ItemActor->ItemRowName.ToString(), EmptySlotIndex);
    return true;
}

void UToolbarInventoryComponent::PostAddProcess()
{
    OnInventoryUpdated.Broadcast();
}

bool UToolbarInventoryComponent::DropCurrentEquippedItem()
{
    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        LOG_Item_WARNING(TEXT("[DropCurrentEquippedItem] 드롭 실패: 장착 슬롯 인덱스가 잘못됨"));
        return false;
    }

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        return UInventoryDropSystem::ExecuteDropItem(this, CurrentEquippedSlotIndex, 1, true);
    }
    else
    {
        Server_DropItem(CurrentEquippedSlotIndex, 1);
        return true;
    }
}

void UToolbarInventoryComponent::Server_DropItem_Implementation(int32 SlotIndex, int32 Quantity)
{
    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        LOG_Item_WARNING(TEXT("[Server_DropItem] 잘못된 슬롯 인덱스: %d"), SlotIndex);
        return;
    }

    bool bIsEquipped = (SlotIndex == CurrentEquippedSlotIndex);

    UInventoryDropSystem::ExecuteDropItem(this, SlotIndex, Quantity, bIsEquipped);
}

bool UToolbarInventoryComponent::TryDropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        LOG_Item_WARNING(TEXT("[TryDropItemAtSlot] 잘못된 슬롯 인덱스: %d"), SlotIndex);
        return false;
    }

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        bool bIsEquipped = (SlotIndex == CurrentEquippedSlotIndex);
        return UInventoryDropSystem::ExecuteDropItem(this, SlotIndex, Quantity, bIsEquipped);
    }
    else
    {
        Server_DropItem(SlotIndex, Quantity);
        return true;
    }
}

void UToolbarInventoryComponent::HandleBackpackEquip(int32 SlotIndex)
{
    if (BackpackManager)
    {
        BackpackManager->EquipBackpack(SlotIndex);
    }
}

void UToolbarInventoryComponent::HandleBackpackUnequip(int32 SlotIndex)
{
    if (BackpackManager)
    {
        BackpackManager->UnequipBackpack(SlotIndex);
    }
}

bool UToolbarInventoryComponent::IsBackpackItem(const FItemDataRow* ItemData) const
{
    return UInventoryUtility::IsBackpackItem(ItemData);
}

bool UToolbarInventoryComponent::IsBackpackItem(FName ItemRowName) const
{
    if (ItemRowName.IsNone())
    {
        return false;
    }

    FItemDataRow* ItemData = UInventoryUtility::GetItemDataByRowName(ItemRowName, ItemDataTable);
    return UInventoryUtility::IsBackpackItem(ItemData);
}

bool UToolbarInventoryComponent::HasOtherEquippedItems() const
{
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (i != CurrentEquippedSlotIndex && ItemSlots[i].bIsEquipped && !IsDefaultItem(ItemSlots[i].ItemRowName))
        {
            return true;
        }
    }
    return false;
}

void UToolbarInventoryComponent::Multicast_SetBackpackVisibility_Implementation(bool bVisible)
{
    if (ABackpackItem* BackpackItem = Cast<ABackpackItem>(EquippedItemComponent->GetChildActor()))
    {
        if (UMeshComponent* MeshComp = BackpackItem->GetMeshComponent())
        {
            MeshComp->SetVisibility(bVisible);
        }
    }
}

bool UToolbarInventoryComponent::IsCollectibleItem(const FItemDataRow* ItemData) const
{
    return UInventoryUtility::IsCollectibleItem(ItemData);
}

TArray<FBackpackSlotData> UToolbarInventoryComponent::GetCurrentBackpackSlots() const
{
    if (CurrentEquippedSlotIndex >= 0 && ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        const FBaseItemSlotData& SlotData = ItemSlots[CurrentEquippedSlotIndex];
        if (SlotData.bIsBackpack)
        {
            return SlotData.BackpackSlots;
        }
    }
    return TArray<FBackpackSlotData>();
}

bool UToolbarInventoryComponent::UpdateCurrentBackpackSlots(const TArray<FBackpackSlotData>& NewSlots)
{
    if (CurrentEquippedSlotIndex >= 0 && ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        FBaseItemSlotData& SlotData = ItemSlots[CurrentEquippedSlotIndex];
        if (SlotData.bIsBackpack)
        {
            SlotData.BackpackSlots = NewSlots;
            OnInventoryUpdated.Broadcast();
            return true;
        }
    }
    return false;
}

bool UToolbarInventoryComponent::AddItemToBackpack(FName ItemRowName, int32 Quantity, int32 BackpackSlotIndex)
{
    return BackpackManager ? BackpackManager->AddItemToBackpack(ItemRowName, Quantity, BackpackSlotIndex) : false;
}

void UToolbarInventoryComponent::OnBackpackEquippedHandler(const TArray<FBackpackSlotData>& BackpackSlots)
{
    if (UIController)
    {
        UIController->ShowBackpackUI(BackpackSlots);
    }
}

void UToolbarInventoryComponent::OnBackpackUnequippedHandler()
{
    if (UIController)
    {
        UIController->HideBackpackUI();
    }
}

bool UToolbarInventoryComponent::RemoveItemFromBackpack(int32 BackpackSlotIndex, int32 Quantity)
{
    if (CurrentEquippedSlotIndex >= 0 && ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        FBaseItemSlotData& SlotData = ItemSlots[CurrentEquippedSlotIndex];
        if (SlotData.bIsBackpack && SlotData.BackpackSlots.IsValidIndex(BackpackSlotIndex))
        {
            FBackpackSlotData& BackpackSlot = SlotData.BackpackSlots[BackpackSlotIndex];

            BackpackSlot.Quantity -= Quantity;

            if (BackpackSlot.Quantity <= 0)
            {
                BackpackSlot.ItemRowName = "Default";
                BackpackSlot.Quantity = 0;
            }

            SyncInventoryToPlayerState();
            OnInventoryUpdated.Broadcast();
            return true;
        }
    }
    return false;
}

bool UToolbarInventoryComponent::HasBackpackEquipped() const
{
    return BackpackManager ? BackpackManager->HasBackpackEquipped() : false;
}

TArray<int32> UToolbarInventoryComponent::GetAllBackpackItemIDs() const
{
    TArray<int32> Result;
    for (const FBaseItemSlotData& Slot : ItemSlots)
    {
        if (Slot.bIsBackpack)
        {
            for (const FBackpackSlotData& BackpackSlot : Slot.BackpackSlots)
            {
                // 빈 아이템 제외 (Default 등)
                if (!UInventoryUtility::IsDefaultItem(BackpackSlot.ItemRowName, GetInventoryConfig()) && BackpackSlot.Quantity > 0)
                {
                    int32 ItemID = GetItemIDFromRowName(BackpackSlot.ItemRowName);
                    Result.Add(ItemID);
                }
            }
        }
    }
    return Result;
}

void UToolbarInventoryComponent::SyncInventoryToPlayerState()
{
    if (IsOwnerCharacterValid())
    {
        if (ABasePlayerState* PS = CachedOwnerCharacter->GetPlayerState<ABasePlayerState>())
        {
            PS->AquiredItemIDs = GetInventoryItemIDs();

            // [디버그 로그] 현재 PS의 AquiredItemIDs를 이름으로 출력
            FString DebugList;
            for (int32 SavedItemID : PS->AquiredItemIDs)
            {
                FName RowName = GetItemRowNameFromID(SavedItemID);
                DebugList += RowName.ToString() + TEXT(", ");
            }
            LOG_Item_WARNING(TEXT("[Sync] PS에 저장된 아이템 목록: %s"), *DebugList);
        }
    }
}

TArray<int32> UToolbarInventoryComponent::GetInventoryItemIDs() const
{
    TArray<int32> ItemIDs;
    ItemIDs.Reserve(ItemSlots.Num());

    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        const FBaseItemSlotData& SlotData = ItemSlots[i];

        int32 ItemID = UInventoryUtility::GetItemIDFromRowName(SlotData.ItemRowName, ItemDataTable);
        ItemIDs.Add(ItemID);
    }

    return ItemIDs;
}

void UToolbarInventoryComponent::SetInventoryFromItemIDs(const TArray<int32>& ItemIDs)
{
    LOG_Item_WARNING(TEXT("[SetInventoryFromItemIDs] === 시작 === 받은 ItemID 수: %d"), ItemIDs.Num());

    // 기존 인벤토리 초기화
    ClearInventorySlots();

    // 슬롯 수를 ItemID 배열 크기로 맞추기
    if (ItemIDs.Num() > MaxSlots)
    {
        LOG_Item_WARNING(TEXT("[SetInventoryFromItemIDs] ItemID 수(%d)가 MaxSlots(%d)보다 큽니다. MaxSlots로 제한합니다."), ItemIDs.Num(), MaxSlots);
    }

    int32 SlotsToRestore = FMath::Min(ItemIDs.Num(), MaxSlots);
    ItemSlots.SetNum(SlotsToRestore);

    // 각 슬롯에 ItemID 기반 데이터 설정
    for (int32 i = 0; i < SlotsToRestore; ++i)
    {
        int32 ItemID = ItemIDs[i];

        // ItemID를 ItemRowName으로 변환
        FName ItemRowName = UInventoryUtility::GetItemRowNameFromID(ItemID, ItemDataTable);

        LOG_Item_WARNING(TEXT("ItemID: %d → ItemRowName: %s"), ItemID, *ItemRowName.ToString());

        if (ItemRowName.IsNone())
        {
            LOG_Item_WARNING(TEXT("[SetInventoryFromItemIDs] 슬롯 %d: ItemID %d에 해당하는 아이템을 찾을 수 없음 -> Default로 설정"), i, ItemID);
            UInventoryUtility::SetSlotToDefault(ItemSlots[i], GetInventoryConfig());
        }
        else
        {
            // 유효한 아이템 설정
            FBaseItemSlotData& SlotData = ItemSlots[i];
            SlotData.ItemRowName = ItemRowName;
            SlotData.Quantity = 1;
            SlotData.Durability = 100.0f;
            SlotData.bIsValid = true;
            SlotData.bIsEquipped = false;

            ULCGameInstanceSubsystem* GameSubsystem = GetOwner()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
            if (GameSubsystem && GameSubsystem->GunDataTable)
            {
                FGunDataRow* GunRowData = GameSubsystem->GunDataTable->FindRow<FGunDataRow>(ItemRowName, TEXT("SetInventoryFromItemIDs"));
                if (GunRowData)
                {
                    SlotData.FireMode = static_cast<int32>(GunRowData->DefaultFireMode);
                    SlotData.bWasAutoFiring = false;
                }
            }

            const FItemDataRow* ItemData = UInventoryUtility::GetItemDataByRowName(ItemRowName, ItemDataTable);
            if (UInventoryUtility::IsBackpackItem(ItemData))
            {
                SlotData.bIsBackpack = true;
                SlotData.BackpackSlots.Empty();

                // 기본 20개 슬롯으로 초기화 (빈 슬롯)
                for (int32 j = 0; j < 20; ++j)
                {
                    FBackpackSlotData DefaultBackpackSlot;
                    DefaultBackpackSlot.ItemRowName = FName("Default");
                    DefaultBackpackSlot.Quantity = 0;
                    SlotData.BackpackSlots.Add(DefaultBackpackSlot);
                }

                LOG_Item_WARNING(TEXT("[SetInventoryFromItemIDs] 가방 아이템 복원: %s (20개 슬롯 초기화)"), *ItemRowName.ToString());
            }

            LOG_Item_WARNING(TEXT("[SetInventoryFromItemIDs] 슬롯 %d: ItemID %d -> %s 복원 성공"), i, ItemID, *ItemRowName.ToString());
        }
    }

    // 남은 슬롯들은 Default로 채우기
    for (int32 i = SlotsToRestore; i < MaxSlots; ++i)
    {
        if (i < ItemSlots.Num())
        {
            SetSlotToDefault(i);
        }
        else
        {
            // 새 슬롯 추가
            FBaseItemSlotData DefaultSlot;
            DefaultSlot.ItemRowName = FName("Default");
            DefaultSlot.Quantity = 1;
            DefaultSlot.Durability = 100.0f;
            DefaultSlot.bIsValid = true;
            DefaultSlot.bIsEquipped = false;
            ItemSlots.Add(DefaultSlot);
        }
    }

    // 무게 갱신 및 UI 새로고침
    UpdateWeight();
    UpdateWalkieTalkieChannelStatus();

    OnInventoryUpdated.Broadcast();

    LOG_Item_WARNING(TEXT("[SetInventoryFromItemIDs] 인벤토리 복원 완료 - 총 %d개 슬롯"), ItemSlots.Num());
}


void UToolbarInventoryComponent::MulticastUpdateItemText_Implementation(const FText& ItemName)
{
    if (UIController)
    {
        UIController->Multicast_UpdateItemText(ItemName);
    }
}

bool UToolbarInventoryComponent::TrySwapBackpackSlots(int32 FromBackpackIndex, int32 ToBackpackIndex)
{
    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        LOG_Item_WARNING(TEXT("[TrySwapBackpackSlots] 가방이 장착되지 않음"));
        return false;
    }

    FBaseItemSlotData& SlotData = ItemSlots[CurrentEquippedSlotIndex];
    if (!SlotData.bIsBackpack)
    {
        LOG_Item_WARNING(TEXT("[TrySwapBackpackSlots] 현재 장착된 아이템이 가방이 아님"));
        return false;
    }

    if (!SlotData.BackpackSlots.IsValidIndex(FromBackpackIndex) ||
        !SlotData.BackpackSlots.IsValidIndex(ToBackpackIndex))
    {
        LOG_Item_WARNING(TEXT("[TrySwapBackpackSlots] 유효하지 않은 가방 슬롯 인덱스: %d, %d"),
            FromBackpackIndex, ToBackpackIndex);
        return false;
    }

    // 가방 슬롯 스왑
    SlotData.BackpackSlots.Swap(FromBackpackIndex, ToBackpackIndex);

    OnInventoryUpdated.Broadcast();

    LOG_Item_WARNING(TEXT("[TrySwapBackpackSlots] 가방 슬롯 스왑 성공: %d <-> %d"),
        FromBackpackIndex, ToBackpackIndex);

    return true;
}

bool UToolbarInventoryComponent::TryMoveToolbarItemToBackpack(int32 ToolbarIndex, int32 BackpackIndex)
{
    if (!ItemSlots.IsValidIndex(ToolbarIndex))
    {
        LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] 유효하지 않은 툴바 인덱스: %d"), ToolbarIndex);
        return false;
    }

    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] 가방이 장착되지 않음"));
        return false;
    }

    FBaseItemSlotData& ToolbarSlot = ItemSlots[ToolbarIndex];
    FBaseItemSlotData& BackpackOwnerSlot = ItemSlots[CurrentEquippedSlotIndex];

    if (!ItemDataTable)
    {
        LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] 데이터 테이블이 없음"));
        return false;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ToolbarSlot.ItemRowName, TEXT("TryMoveToolbarItemToBackpack"));
    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] 데이터가 유효하지 않음"));
        return false;
    }

    // Collectible이 아니면 가방 이동 불가
    if (!UInventoryUtility::IsCollectibleItem(ItemData))
    {
        LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] 가방에 넣을 수 없는(Collectible 아님) 아이템: %s"), *ToolbarSlot.ItemRowName.ToString());
        return false;
    }

    if (!BackpackOwnerSlot.bIsBackpack || !BackpackOwnerSlot.BackpackSlots.IsValidIndex(BackpackIndex))
    {
        LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] 유효하지 않은 가방 슬롯: %d"), BackpackIndex);
        return false;
    }

    // Default 아이템은 이동 불가
    if (UInventoryUtility::IsDefaultItem(ToolbarSlot.ItemRowName, GetInventoryConfig()))
    {
        LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] Default 아이템은 이동할 수 없음"));
        return false;
    }

    FBackpackSlotData& BackpackSlot = BackpackOwnerSlot.BackpackSlots[BackpackIndex];

    // 가방 슬롯이 비어있는지 확인
    if (!UInventoryUtility::IsDefaultItem(BackpackSlot.ItemRowName, GetInventoryConfig()) && BackpackSlot.Quantity > 0)
    {
        LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] 대상 가방 슬롯이 비어있지 않음"));
        return false;
    }

    // 아이템 이동
    BackpackSlot.ItemRowName = ToolbarSlot.ItemRowName;
    BackpackSlot.Quantity = ToolbarSlot.Quantity;

    // 툴바 슬롯을 Default로 설정
    SetSlotToDefault(ToolbarIndex);

    OnInventoryUpdated.Broadcast();

    LOG_Item_WARNING(TEXT("[TryMoveToolbarItemToBackpack] 툴바->가방 이동 성공: %s"), *BackpackSlot.ItemRowName.ToString());

    return true;
}

bool UToolbarInventoryComponent::TryMoveBackpackItemToToolbar(int32 BackpackIndex, int32 ToolbarIndex)
{
    if (!ItemSlots.IsValidIndex(ToolbarIndex))
    {
        LOG_Item_WARNING(TEXT("[TryMoveBackpackItemToToolbar] 유효하지 않은 툴바 인덱스: %d"), ToolbarIndex);
        return false;
    }

    if (CurrentEquippedSlotIndex < 0 || !ItemSlots.IsValidIndex(CurrentEquippedSlotIndex))
    {
        LOG_Item_WARNING(TEXT("[TryMoveBackpackItemToToolbar] 가방이 장착되지 않음"));
        return false;
    }

    FBaseItemSlotData& ToolbarSlot = ItemSlots[ToolbarIndex];
    FBaseItemSlotData& BackpackOwnerSlot = ItemSlots[CurrentEquippedSlotIndex];

    if (!BackpackOwnerSlot.bIsBackpack || !BackpackOwnerSlot.BackpackSlots.IsValidIndex(BackpackIndex))
    {
        LOG_Item_WARNING(TEXT("[TryMoveBackpackItemToToolbar] 유효하지 않은 가방 슬롯: %d"), BackpackIndex);
        return false;
    }

    FBackpackSlotData& BackpackSlot = BackpackOwnerSlot.BackpackSlots[BackpackIndex];

    // 가방 슬롯이 비어있으면 이동 불가
    if (UInventoryUtility::IsDefaultItem(BackpackSlot.ItemRowName, GetInventoryConfig()) || BackpackSlot.Quantity <= 0)
    {
        LOG_Item_WARNING(TEXT("[TryMoveBackpackItemToToolbar] 소스 가방 슬롯이 비어있음"));
        return false;
    }

    // 툴바 슬롯이 Default가 아니면 이동 불가 (스왑은 별도 구현 필요)
    if (!UInventoryUtility::IsDefaultItem(ToolbarSlot.ItemRowName, GetInventoryConfig()))
    {
        LOG_Item_WARNING(TEXT("[TryMoveBackpackItemToToolbar] 대상 툴바 슬롯이 비어있지 않음"));
        return false;
    }

    // 아이템 이동
    ToolbarSlot.ItemRowName = BackpackSlot.ItemRowName;
    ToolbarSlot.Quantity = BackpackSlot.Quantity;
    ToolbarSlot.bIsValid = true;
    ToolbarSlot.bIsEquipped = false;

    // 가방 슬롯을 Default로 설정
    BackpackSlot.ItemRowName = FName("Default");
    BackpackSlot.Quantity = 0;

    OnInventoryUpdated.Broadcast();

    LOG_Item_WARNING(TEXT("[TryMoveBackpackItemToToolbar] 가방->툴바 이동 성공: %s"), *ToolbarSlot.ItemRowName.ToString());

    return true;
}
