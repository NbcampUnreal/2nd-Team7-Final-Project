#include "Inventory/InventoryComponentBase.h"
#include "DataType/InventoryItemData.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"


UInventoryComponentBase::UInventoryComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponentBase::BeginPlay()
{
	Super::BeginPlay();

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

	// 툴팁 위젯은 아마도 UI 매니저에서 관리하게 될듯?

	ItemSlots.Reserve(MaxSlots);
}

bool UInventoryComponentBase::AddItem(FName ItemRowName, int32 Amount)
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
	}

	int32 RemainAmount = Amount;

	for (FItemSlot& Slot : ItemSlots)
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

		FItemSlot NewSlot;
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
		return false;
	}
}

bool UInventoryComponentBase::DecreaseItem(FName ItemRowName, int32 Amount)
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

int32 UInventoryComponentBase::GetItemCount(FName ItemRowName) const
{
	int32 TotalCount = 0;

	for (const FItemSlot& Slot : ItemSlots)
	{
		if (Slot.ItemRowName == ItemRowName)
		{
			TotalCount += Slot.Quantity;
		}
	}

	return TotalCount;
}

bool UInventoryComponentBase::SwapItemSlots(int32 FromIndex, int32 ToIndex)
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

bool UInventoryComponentBase::RemoveItemAtSlot(int32 SlotIndex)
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

bool UInventoryComponentBase::TryAddItem(AItemBase* ItemActor)
{
	if (!PreAddCheck(ItemActor))
	{
		return false;
	}

	if (!StoreItem(ItemActor))
	{
		return false;
	}

	PostAddProcess(ItemActor);

	return true;
}

// 아래 함수는 UI매니저에서 관리해야할 수 있으므로 보류
void UInventoryComponentBase::ShowTooltipForItem(const FInventoryItemData& ItemData, UWidget* TargetWidget)
{
}

void UInventoryComponentBase::HideTooltip()
{
}

