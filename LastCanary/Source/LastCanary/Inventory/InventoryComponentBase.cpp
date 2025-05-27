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
    if (!IsOwnerCharacterValid())
    {
        LOG_Item_WARNING(TEXT("[CalculateDropLocation] CachedOwnerCharacter가 유효하지 않습니다."));
        return FVector::ZeroVector;
    }

    // ⭐ ItemSpawner 설정값 사용
    if (!ItemSpawner)
    {
        return CachedOwnerCharacter->GetActorLocation() + CachedOwnerCharacter->GetActorForwardVector() * 200.0f;
    }

    // 캐릭터의 위치와 방향 가져오기
    FVector CharacterLocation = CachedOwnerCharacter->GetActorLocation();
    FVector ForwardVector = CachedOwnerCharacter->GetActorForwardVector();

    // ItemSpawner의 설정값을 사용하여 위치 계산
    float DropDistance = 200.0f; // ItemSpawner에서 설정 가능하도록 수정 예정
    FVector DropLocation = CharacterLocation + ForwardVector * DropDistance;

    // 높이 오프셋 적용
    DropLocation.Z += 50.0f;

    // 바닥에 라인 트레이스로 정확한 위치 찾기
    FHitResult HitResult;
    FVector TraceStart = DropLocation + FVector(0, 0, 500.0f);
    FVector TraceEnd = DropLocation - FVector(0, 0, 1000.0f);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(CachedOwnerCharacter);
    QueryParams.bTraceComplex = true;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        DropLocation = HitResult.ImpactPoint + FVector(0, 0, 10.0f);
    }

    return DropLocation;
}

bool UInventoryComponentBase::TryDropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
    // ⭐ 서버라면 직접 실행, 클라이언트라면 RPC 호출
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        // 서버에서 직접 실행
        UE_LOG(LogTemp, Warning, TEXT("[TryDropItemAtSlot] 서버에서 직접 실행: 슬롯 %d"), SlotIndex);
        return Internal_TryDropItemAtSlot(SlotIndex, Quantity);
    }
    else
    {
        // 클라이언트에서 서버 RPC 호출
        UE_LOG(LogTemp, Warning, TEXT("[TryDropItemAtSlot] 클라이언트에서 서버 RPC 호출: 슬롯 %d"), SlotIndex);
        Server_TryDropItemAtSlot(SlotIndex, Quantity);
        return true; // RPC 호출 성공 (실제 결과는 서버에서 처리)
    }
}

void UInventoryComponentBase::Server_TryDropItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity)
{
    UE_LOG(LogTemp, Warning, TEXT("[Server_TryDropItemAtSlot] 서버에서 실행: 슬롯 %d"), SlotIndex);

    // 서버에서 실제 드랍 로직 실행
    Internal_TryDropItemAtSlot(SlotIndex, Quantity);
}

bool UInventoryComponentBase::Internal_TryDropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[TryDropItemAtSlot_Internal] Authority가 없습니다. 서버에서만 실행하세요."));
        return false;
    }

    if (!ItemSlots.IsValidIndex(SlotIndex))
    {
        LOG_Item_WARNING(TEXT("[TryDropItemAtSlot_Internal] 유효하지 않은 슬롯 인덱스: %d"), SlotIndex);
        return false;
    }

    if (!IsOwnerCharacterValid())
    {
        LOG_Item_WARNING(TEXT("[TryDropItemAtSlot_Internal] CachedOwnerCharacter가 유효하지 않습니다."));
        return false;
    }

    FBaseItemSlotData& SlotData = ItemSlots[SlotIndex];

    // 빈 슬롯인지 확인
    if (SlotData.ItemRowName.IsNone() || SlotData.Quantity <= 0)
    {
        LOG_Item_WARNING(TEXT("[TryDropItemAtSlot_Internal] 빈 슬롯입니다: %d"), SlotIndex);
        return false;
    }

    // 드랍할 수량 확인
    int32 DropQuantity = FMath::Min(Quantity, SlotData.Quantity);
    if (DropQuantity <= 0)
    {
        LOG_Item_WARNING(TEXT("[TryDropItemAtSlot_Internal] 드랍할 수량이 0 이하입니다."));
        return false;
    }

    // 드랍 위치 계산
    FVector DropLocation = CalculateDropLocation();

    // 드랍할 아이템 데이터 준비
    FBaseItemSlotData DropItemData = SlotData;
    DropItemData.Quantity = DropQuantity;

    // ⭐ ItemSpawner를 이용한 아이템 생성
    AItemBase* DroppedItem = ItemSpawner->CreateItemFromData(DropItemData, DropLocation);
    if (!DroppedItem)
    {
        LOG_Item_WARNING(TEXT("[TryDropItemAtSlot_Internal] 아이템 스폰 실패"));
        return false;
    }

    // 인벤토리에서 아이템 제거
    SlotData.Quantity -= DropQuantity;
    if (SlotData.Quantity <= 0)
    {
        // 슬롯 초기화
        SlotData = FBaseItemSlotData();
    }

    // UI 업데이트 (복제되므로 모든 클라이언트에 자동 전파)
    OnInventoryUpdated.Broadcast();

    LOG_Item_WARNING(TEXT("[TryDropItemAtSlot_Internal] 아이템 드랍 성공: %s (수량: %d)"),
        *DropItemData.ItemRowName.ToString(), DropQuantity);

    return true;
}

bool UInventoryComponentBase::TryDropItem(FName ItemRowName, int32 Quantity)
{
    // ⭐ 서버라면 직접 실행, 클라이언트라면 RPC 호출
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        // 서버에서 직접 실행
        UE_LOG(LogTemp, Warning, TEXT("[TryDropItem] 서버에서 직접 실행: %s"), *ItemRowName.ToString());
        return TryDropItem_Internal(ItemRowName, Quantity);
    }
    else
    {
        // 클라이언트에서 서버 RPC 호출
        UE_LOG(LogTemp, Warning, TEXT("[TryDropItem] 클라이언트에서 서버 RPC 호출: %s"), *ItemRowName.ToString());
        Server_TryDropItem(ItemRowName, Quantity);
        return true; // RPC 호출 성공
    }
}

void UInventoryComponentBase::Server_TryDropItem_Implementation(FName ItemRowName, int32 Quantity)
{
    UE_LOG(LogTemp, Warning, TEXT("[Server_TryDropItem] 서버에서 실행: %s"), *ItemRowName.ToString());

    // 서버에서 실제 드랍 로직 실행
    TryDropItem_Internal(ItemRowName, Quantity);
}

bool UInventoryComponentBase::TryDropItem_Internal(FName ItemRowName, int32 Quantity)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[TryDropItem_Internal] Authority가 없습니다."));
        return false;
    }

    if (ItemRowName.IsNone() || Quantity <= 0)
    {
        LOG_Item_WARNING(TEXT("[TryDropItem_Internal] 유효하지 않은 매개변수"));
        return false;
    }

    // 해당 아이템을 가진 슬롯 찾기
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (ItemSlots[i].ItemRowName == ItemRowName && ItemSlots[i].Quantity > 0)
        {
            int32 DropQuantity = FMath::Min(Quantity, ItemSlots[i].Quantity);
            return Internal_TryDropItemAtSlot(i, DropQuantity);
        }
    }

    LOG_Item_WARNING(TEXT("[TryDropItem_Internal] 아이템을 찾을 수 없습니다: %s"), *ItemRowName.ToString());
    return false;
}