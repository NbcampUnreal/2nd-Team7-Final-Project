#include "Item/EquipmentItem/EquipmentItemBase.h"

AEquipmentItemBase::AEquipmentItemBase()
{
}

void AEquipmentItemBase::UseItem()
{
    //Super::UseItem();
    //OnItemStateChanged.Broadcast();
}

void AEquipmentItemBase::SetEquipped(bool bNewEquipped)
{
    bIsEquipped = bNewEquipped;

    // 장비 상태가 변경되었을 때 알림 발생
    OnItemStateChanged.Broadcast();
}
