// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/BackpackInventoryComponent.h"
#include "LastCanary.h"

UBackpackInventoryComponent::UBackpackInventoryComponent()
{
	MaxSlots = 20;
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

	if (ItemSlots.Num() >= MaxSlots)
	{
		return false;
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

	for (FItemSlot& Slot : ItemSlots)
	{
		if (Slot.ItemRowName == ItemActor->ItemRowName)
		{
			int32 StackSpace = ItemData->MaxStack - Slot.Quantity;
			int32 Addable = FMath::Min(1, StackSpace);
			if (Addable > 0)
			{
				Slot.Quantity += Addable;
				ItemActor->Destroy();
				return true;
			}
		}
	}

	if (ItemSlots.Num() < MaxSlots)
	{
		FItemSlot NewSlot;
		NewSlot.ItemRowName = ItemActor->ItemRowName;
		NewSlot.Quantity = 1;
		ItemSlots.Add(NewSlot);

		ItemActor->Destroy();
		return true;
	}

	LOG_Item_WARNING(TEXT("[BackpackInventoryComponent::TryStoreItem] 저장 공간이 부족합니다."));
	// TODO : 저장할 공간이 부족하다는 위젯 또는 메시지 출력이 필요할지도
	return false;
}

void UBackpackInventoryComponent::PostAddProcess()
{
	OnInventoryUpdated.Broadcast();
}
