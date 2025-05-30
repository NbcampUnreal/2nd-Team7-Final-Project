#include "Inventory/InventoryComponentBase.h"
#include "Item/ItemSpawnerComponent.h"
#include "Character/BaseCharacter.h"
#include "DataType/BaseItemSlotData.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

UInventoryComponentBase::UInventoryComponentBase()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    ItemSpawner = CreateDefaultSubobject<UItemSpawnerComponent>(TEXT("ItemSpawner"));
}

void UInventoryComponentBase::BeginPlay()
{
    Super::BeginPlay();

    CacheOwnerCharacter();

    UWorld* World = GetWorld();
    if (!World)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] World is null!"));
        return;
    }

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] GameInstance is null!"));
        return;
    }

    ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!GISubsystem)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] LCGameInstance is null"));
        return;
    }

    ItemDataTable = GISubsystem->ItemDataTable;
    if (!ItemDataTable)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] ItemDataTable is null"));
        return;
    }

    // 서버에서만 슬롯 초기화
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        InitializeSlots();
    }
}

void UInventoryComponentBase::InitializeSlots()
{
    // 이미 초기화되었다면 건너뛰기
    if (ItemSlots.Num() > 0)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::InitializeSlots] 슬롯이 이미 초기화되어 있습니다."));
        return;
    }

    // MaxSlots 크기만큼 빈 슬롯 생성
    ItemSlots.Empty();
    ItemSlots.Reserve(MaxSlots);

    for (int32 i = 0; i < MaxSlots; ++i)
    {
        FBaseItemSlotData EmptySlot;
        ItemSlots.Add(EmptySlot);
    }

    // UI 업데이트
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponentBase::CacheOwnerCharacter()
{
    CachedOwnerCharacter = Cast<ABaseCharacter>(GetOwner());
    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::CacheOwnerCharacter] 소유자가 BaseCharacter가 아닙니다."));
    }
}

bool UInventoryComponentBase::IsOwnerCharacterValid() const
{
    return CachedOwnerCharacter && IsValid(CachedOwnerCharacter);
}

ABaseCharacter* UInventoryComponentBase::GetCachedOwnerCharacter() const
{
    return CachedOwnerCharacter;
}

void UInventoryComponentBase::ShowTooltipForItem(const FBaseItemSlotData& ItemData, UWidget* TargetWidget)
{
    // UI 매니저에서 관리할 예정
}

void UInventoryComponentBase::HideTooltip()
{
    // UI 매니저에서 관리할 예정
}

void UInventoryComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryComponentBase, ItemSlots);
}

void UInventoryComponentBase::OnRep_ItemSlots()
{
    OnInventoryUpdated.Broadcast();
}

FItemDataRow* UInventoryComponentBase::GetItemRowByName(FName RowName)
{
    if (!ItemDataTable)
    {
        LOG_Item_ERROR(TEXT("[InventoryComponentBase::GetItemRowByName] ItemDataTable이 설정되지 않았습니다."));
        return nullptr;
    }

    if (RowName.IsNone())
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::GetItemRowByName] RowName이 None입니다."));
        return nullptr;
    }

    FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("GetItemRowByName"));

    if (!ItemData)
    {
        LOG_Item_WARNING(TEXT("[InventoryComponentBase::GetItemRowByName] '%s' 행을 찾을 수 없습니다."), *RowName.ToString());
        return nullptr;
    }

    return ItemData;
}

int32 UInventoryComponentBase::GetMaxSlots() const
{
    return MaxSlots;
}

FVector UInventoryComponentBase::CalculateDropLocation() const
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        LOG_Item_WARNING(TEXT("[CalculateDropLocation] Owner is null"));
        return FVector::ZeroVector;
    }

    ABaseCharacter* Character = Cast<ABaseCharacter>(OwnerActor);
    if (!Character)
    {
        LOG_Item_WARNING(TEXT("[CalculateDropLocation] Owner is not BaseCharacter"));
        return FVector::ZeroVector;
    }

    // ⭐ 카메라 시점 기준으로 던지기 위치 계산
    AController* Controller = Character->GetController();
    if (!Controller)
    {
        LOG_Item_WARNING(TEXT("[CalculateDropLocation] Controller is null"));
        return Character->GetActorLocation();
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // ⭐ 카메라 앞쪽 일정 거리에서 생성 (손에서 던지는 느낌)
    FVector ThrowStartOffset = CameraRotation.Vector() * 150.0f; // 카메라 앞 150cm
    FVector HandOffset = Character->GetActorUpVector() * -20.0f; // 살짝 아래쪽 (손 높이)

    FVector ThrowStartLocation = CameraLocation + ThrowStartOffset + HandOffset;

    // 바닥과 너무 가까우면 조정
    FHitResult HitResult;
    FVector TraceStart = ThrowStartLocation + FVector(0, 0, 100.0f);
    FVector TraceEnd = ThrowStartLocation - FVector(0, 0, 200.0f);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    QueryParams.bTraceComplex = true;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        float MinHeight = HitResult.ImpactPoint.Z + 80.0f; // 바닥에서 80cm 위
        ThrowStartLocation.Z = FMath::Max(ThrowStartLocation.Z, MinHeight);
    }

    LOG_Item_WARNING(TEXT("[CalculateDropLocation] 던지기 시작 위치: %s"), *ThrowStartLocation.ToString());
    return ThrowStartLocation;
}

bool UInventoryComponentBase::TryDropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
    return Internal_TryDropItemAtSlot(SlotIndex, Quantity);
}

void UInventoryComponentBase::Server_TryDropItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity)
{
    Internal_TryDropItemAtSlot(SlotIndex, Quantity);
}

bool UInventoryComponentBase::Internal_TryDropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] Owner is null"));
        return false;
    }

    if (!Owner->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] Authority가 없습니다. 서버 RPC를 통해 요청하세요."));
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] Owner: %s, HasAuthority: %s"),
            *Owner->GetName(), Owner->HasAuthority() ? TEXT("true") : TEXT("false"));

        // ⭐ 클라이언트에서는 자동으로 서버 RPC 호출
        Server_TryDropItemAtSlot(SlotIndex, Quantity);
        return true; // 클라이언트에서는 요청 성공으로 처리
    }

    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] 유효하지 않은 슬롯 인덱스: %d"), SlotIndex);
        return false;
    }

    if (!IsOwnerCharacterValid())
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] CachedOwnerCharacter가 유효하지 않습니다."));
        return false;
    }

    FBaseItemSlotData& SlotData = ItemSlots[SlotIndex];

    if (SlotData.ItemRowName.IsNone() || SlotData.Quantity <= 0)
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] 빈 슬롯입니다: %d"), SlotIndex);
        return false;
    }

    int32 DropQuantity = FMath::Min(Quantity, SlotData.Quantity);
    if (DropQuantity <= 0)
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] 드랍할 수량이 0 이하입니다."));
        return false;
    }

    FVector DropLocation = CalculateDropLocation();

    FBaseItemSlotData DropItemData = SlotData;
    DropItemData.Quantity = DropQuantity;

    if (!ItemSpawner)
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] ItemSpawner is null"));
        return false;
    }

    AItemBase* DroppedItem = ItemSpawner->CreateItemFromData(DropItemData, DropLocation);
    if (!DroppedItem)
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::Internal_TryDropItemAtSlot] 아이템 스폰 실패"));
        return false;
    }

    SlotData.Quantity -= DropQuantity;
    if (SlotData.Quantity <= 0)
    {
        SlotData = FBaseItemSlotData();
    }

    OnInventoryUpdated.Broadcast();

    return true;
}

bool UInventoryComponentBase::TryDropItem(FName ItemRowName, int32 Quantity)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        return TryDropItem_Internal(ItemRowName, Quantity);
    }
    else
    {
        Server_TryDropItem(ItemRowName, Quantity);
        return true;
    }
}

void UInventoryComponentBase::Server_TryDropItem_Implementation(FName ItemRowName, int32 Quantity)
{
    TryDropItem_Internal(ItemRowName, Quantity);
}

bool UInventoryComponentBase::TryDropItem_Internal(FName ItemRowName, int32 Quantity)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::TryDropItem_Internal] Authority가 없습니다."));
        return false;
    }

    if (ItemRowName.IsNone() || Quantity <= 0)
    {
        LOG_Item_WARNING(TEXT("[UInventoryComponentBase::TryDropItem_Internal] 유효하지 않은 매개변수"));
        return false;
    }

    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (ItemSlots[i].ItemRowName == ItemRowName && ItemSlots[i].Quantity > 0)
        {
            int32 DropQuantity = FMath::Min(Quantity, ItemSlots[i].Quantity);
            return Internal_TryDropItemAtSlot(i, DropQuantity);
        }
    }

    LOG_Item_WARNING(TEXT("[UInventoryComponentBase::TryDropItem_Internal] 아이템을 찾을 수 없습니다: %s"), *ItemRowName.ToString());
    return false;
}
