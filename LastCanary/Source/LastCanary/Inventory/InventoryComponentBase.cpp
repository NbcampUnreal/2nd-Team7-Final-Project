#include "Inventory/InventoryComponentBase.h"
#include "Character/BaseCharacter.h"
#include "DataType/BaseItemSlotData.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"


UInventoryComponentBase::UInventoryComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
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

	// 툴팁 위젯은 아마도 UI 매니저에서 관리하게 될듯?
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
		// EmptySlot은 기본값으로 bIsValid = false
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

// 아래 함수는 UI매니저에서 관리해야할 수 있으므로 보류
void UInventoryComponentBase::ShowTooltipForItem(const FBaseItemSlotData& ItemData, UWidget* TargetWidget)
{
}

void UInventoryComponentBase::HideTooltip()
{
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
