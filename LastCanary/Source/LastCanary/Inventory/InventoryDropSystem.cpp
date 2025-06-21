#include "Inventory/InventoryDropSystem.h"
#include "Inventory/InventoryComponentBase.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/InventoryUtility.h"
#include "Inventory/InventoryConfig.h"
#include "Item/ItemSpawnerComponent.h"
#include "Character/BaseCharacter.h"

bool UInventoryDropSystem::ExecuteDropItem(UInventoryComponentBase* Inventory, int32 SlotIndex, int32 Quantity, bool bIsEquipped)
{
    if (!Inventory || !Inventory->GetOwner())
    {
        return false;
    }

    // 서버에서만 실행
    if (!Inventory->GetOwner()->HasAuthority())
    {
        return false; // 클라이언트는 각자의 RPC 호출 방식 사용
    }

    // 검증
    if (!ValidateDropRequest(Inventory, SlotIndex, Quantity))
    {
        return false;
    }

    // 장착된 아이템 특별 처리
    if (bIsEquipped)
    {
        if (UToolbarInventoryComponent* Toolbar = Cast<UToolbarInventoryComponent>(Inventory))
        {
            HandleEquippedItemDrop(Toolbar, SlotIndex);
        }
    }

    // 드롭 데이터 준비
    FBaseItemSlotData& SlotData = Inventory->ItemSlots[SlotIndex];
    FBaseItemSlotData DropItemData = SlotData;
    DropItemData.Quantity = FMath::Min(Quantity, SlotData.Quantity);
    DropItemData.bIsEquipped = false;

    // 실제 드롭 실행
    return ExecuteDropLogic(Inventory, SlotIndex, Quantity, DropItemData);
}

bool UInventoryDropSystem::ExecuteDropItemByName(UInventoryComponentBase* Inventory, FName ItemRowName, int32 Quantity)
{
    if (!Inventory || ItemRowName.IsNone() || Quantity <= 0)
    {
        return false;
    }

    // 해당 아이템을 가진 슬롯 찾기
    for (int32 i = 0; i < Inventory->ItemSlots.Num(); ++i)
    {
        if (Inventory->ItemSlots[i].ItemRowName == ItemRowName && Inventory->ItemSlots[i].Quantity > 0)
        {
            int32 DropQuantity = FMath::Min(Quantity, Inventory->ItemSlots[i].Quantity);
            return ExecuteDropItem(Inventory, i, DropQuantity);
        }
    }

    return false;
}

bool UInventoryDropSystem::ValidateDropRequest(UInventoryComponentBase* Inventory, int32 SlotIndex, int32 Quantity)
{
    if (!Inventory || !Inventory->ItemSlots.IsValidIndex(SlotIndex))
    {
        return false;
    }

    const FBaseItemSlotData& SlotData = Inventory->ItemSlots[SlotIndex];

    // Default 아이템 체크
    if (UInventoryUtility::IsDefaultItem(SlotData.ItemRowName))
    {
        return false;
    }

    if (SlotData.ItemRowName.IsNone() || SlotData.Quantity <= 0 || Quantity > SlotData.Quantity)
    {
        return false;
    }

    return true;
}

FVector UInventoryDropSystem::CalculateDropLocation(AActor* Owner, const UInventoryConfig* Config)
{
    if (!Owner)
    {
        return FVector::ZeroVector;
    }

    ABaseCharacter* Character = Cast<ABaseCharacter>(Owner);
    if (!Character)
    {
        return Owner->GetActorLocation();
    }

    AController* Controller = Character->GetController();
    if (!Controller)
    {
        return Character->GetActorLocation();
    }

    float DropDistance = Config ? Config->DropDistanceFromPlayer : 150.0f;
    float MinHeight = Config ? Config->MinDropHeight : 80.0f;

    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector ThrowStartOffset = CameraRotation.Vector() * DropDistance;
    FVector HandOffset = Character->GetActorUpVector() * -20.0f;
    FVector ThrowStartLocation = CameraLocation + ThrowStartOffset + HandOffset;

    // 바닥 체크
    FHitResult HitResult;
    FVector TraceStart = ThrowStartLocation + FVector(0, 0, 100.0f);
    FVector TraceEnd = ThrowStartLocation - FVector(0, 0, 200.0f);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    QueryParams.bTraceComplex = true;

    if (Owner->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        float MinHeightFromGround = HitResult.ImpactPoint.Z + MinHeight;
        ThrowStartLocation.Z = FMath::Max(ThrowStartLocation.Z, MinHeightFromGround);
    }

    return ThrowStartLocation;
}

void UInventoryDropSystem::HandleEquippedItemDrop(UToolbarInventoryComponent* Toolbar, int32 SlotIndex)
{
    if (!Toolbar || SlotIndex != Toolbar->GetCurrentEquippedSlotIndex())
    {
        return;
    }

    // 장착된 아이템의 내구도를 슬롯에 동기화
    Toolbar->SyncEquippedItemDurabilityToSlot();

    // 총기 상태를 슬롯에 동기화 (발사 모드, 자동 발사 상태 등)
    Toolbar->SyncGunStateToSlot();

    // 현재 장착된 아이템 해제
    Toolbar->UnequipCurrentItem();
}

bool UInventoryDropSystem::ExecuteDropLogic(UInventoryComponentBase* Inventory, int32 SlotIndex, int32 Quantity, const FBaseItemSlotData& DropItemData)
{
    UItemSpawnerComponent* ItemSpawner = Inventory->GetItemSpawner();
    if (!ItemSpawner)
    {
        return false;
    }

    FVector DropLocation = CalculateDropLocation(Inventory->GetOwner(), Inventory->GetInventoryConfig());

    // 아이템 생성
    AItemBase* DroppedItem = ItemSpawner->CreateItemFromData(DropItemData, DropLocation);
    if (!DroppedItem)
    {
        return false;
    }

    // 특별한 아이템 타입 처리
    if (AGunBase* DroppedGun = Cast<AGunBase>(DroppedItem))
    {
        DroppedGun->CurrentFireMode = static_cast<EFireMode>(DropItemData.FireMode);
        DroppedGun->bIsAutoFiring = false;
    }

    if (ABackpackItem* DroppedBackpack = Cast<ABackpackItem>(DroppedItem))
    {
        DroppedBackpack->SetBackpackData(DropItemData.BackpackSlots);
    }

    // 슬롯 업데이트
    FBaseItemSlotData& SlotData = Inventory->ItemSlots[SlotIndex];
    SlotData.Quantity -= DropItemData.Quantity;

    if (SlotData.Quantity <= 0)
    {
        UInventoryUtility::SetSlotToDefault(SlotData, Inventory->GetInventoryConfig());
    }

    Inventory->UpdateWeight();
    Inventory->UpdateWalkieTalkieChannelStatus();
    Inventory->OnInventoryUpdated.Broadcast();

    return true;
}
