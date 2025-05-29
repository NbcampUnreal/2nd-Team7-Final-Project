#include "Item/EquipmentItem/BackpackItem.h"

UBackpackInventoryComponent* ABackpackItem::GetBackpackInventoryComponent() const
{
	return BackpackInventoryComponent;
}

void ABackpackItem::CopyInventoryData(UBackpackInventoryComponent* NewBackpackInventoryComponent)
{
	if (NewBackpackInventoryComponent)
	{
		// TODO : 기존 인벤토리 데이터에 새로운 가방의 데이터를 추가하거나 하는 방식이 필요할 듯
	}
}
