// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/BackpackInventoryComponent.h"
#include "Item/ItemBase.h"
#include "LastCanary.h"

UBackpackInventoryComponent::UBackpackInventoryComponent()
{
	MaxSlots = 20;
}

void UBackpackInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ItemSlots.Reserve(MaxSlots);
}

bool UBackpackInventoryComponent::TryAddItemSlot(FName ItemRowName, int32 Amount)
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

bool UBackpackInventoryComponent::TryDecreaseItem(FName ItemRowName, int32 Amount)
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

int32 UBackpackInventoryComponent::GetItemCount(FName ItemRowName) const
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

bool UBackpackInventoryComponent::TrySwapItemSlots(int32 FromIndex, int32 ToIndex)
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

bool UBackpackInventoryComponent::TryRemoveItemAtSlot(int32 SlotIndex)
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

bool UBackpackInventoryComponent::TryAddItem(AItemBase* ItemActor)
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

bool UBackpackInventoryComponent::CanAddItem(AItemBase* ItemActor)
{
	if (!ItemActor)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::CanAddItem] ItemActor is null!"));
		return false;
	}

	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::CanAddItem] ItemDataTable is null!"));
		return false;
	}
	
	const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemActor->ItemRowName, TEXT("CanAddItem"));
	if (!ItemData)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::CanAddItem] ItemData is null!"));
		return false;
	}

	const FGameplayTag CollectibleTag = FGameplayTag::RequestGameplayTag(FName("ItemType.Collectible"));
	if (!ItemData->ItemType.MatchesTag(CollectibleTag))
	{
		return false;
	}

	LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::CanAddItem] 슬롯 가용성 확인 시작 (총 %d개 슬롯)"), ItemSlots.Num());

	// 1. 스택 가능한 기존 슬롯 확인
	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		const FBaseItemSlotData& Slot = ItemSlots[i];

		// 빈 슬롯 확인
		if (Slot.ItemRowName.IsNone() || Slot.Quantity <= 0)
		{
			LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::CanAddItem] ✅ 빈 슬롯 발견: 인덱스 %d"), i);
			return true;
		}

		// 스택 가능한 슬롯 확인
		if (Slot.ItemRowName == ItemActor->ItemRowName)
		{
			int32 StackSpace = ItemData->MaxStack - Slot.Quantity;
			if (StackSpace > 0)
			{
				LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::CanAddItem] ✅ 스택 가능한 슬롯 발견: 인덱스 %d (여유 공간: %d)"), i, StackSpace);
				return true;
			}
		}
	}

	// TODO : 무게와 같은 제한 조건 추가가 필요할지도

	return true;
}

bool UBackpackInventoryComponent::TryStoreItem(AItemBase* ItemActor)
{
	if (!ItemActor)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] ItemActor is null!"));
		return false;
	}

	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] ItemDataTable is null!"));
		return false;
	}

	const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemActor->ItemRowName, TEXT("TryStoreItem"));
	if (!ItemData)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] ItemData is null!"));
		return false;
	}

	// 1. 기존 슬롯에 스택 시도
	LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] 기존 슬롯 스택 확인 시작"));
	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		FBaseItemSlotData& Slot = ItemSlots[i];

		if (Slot.ItemRowName == ItemActor->ItemRowName && Slot.Quantity > 0)
		{
			int32 StackSpace = ItemData->MaxStack - Slot.Quantity;
			if (StackSpace > 0)
			{
				Slot.Quantity += 1;
				LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] ✅ 기존 슬롯에 스택 성공! 슬롯 %d, 새 수량: %d"), i, Slot.Quantity);
				ItemActor->Destroy();
				return true;
			}
		}
	}

	// 2. 빈 슬롯에 새로 추가
	LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] 빈 슬롯 찾기 시작"));
	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		FBaseItemSlotData& Slot = ItemSlots[i];

		if (Slot.ItemRowName.IsNone() || Slot.Quantity <= 0)
		{
			Slot.ItemRowName = ItemActor->ItemRowName;
			Slot.Quantity = 1;
			LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] ✅ 빈 슬롯에 저장 성공! 슬롯 %d"), i);
			ItemActor->Destroy();
			return true;
		}
	}


	LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] 저장 공간이 부족합니다."));
	// TODO : 저장할 공간이 부족하다는 위젯 또는 메시지 출력이 필요할지도
	return false;
}

void UBackpackInventoryComponent::PostAddProcess()
{
	OnInventoryUpdated.Broadcast();
}
