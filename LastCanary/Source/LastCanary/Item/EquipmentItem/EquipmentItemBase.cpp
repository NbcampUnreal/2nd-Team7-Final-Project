#include "Item/EquipmentItem/EquipmentItemBase.h"

AEquipmentItemBase::AEquipmentItemBase()
{
}

void AEquipmentItemBase::UseItem()
{
    PlaySoundByType();
}

void AEquipmentItemBase::SetEquipped(bool bNewEquipped)
{
    bIsEquipped = bNewEquipped;

    if (UStaticMeshComponent* StaticMesh = this->FindComponentByClass<UStaticMeshComponent>())
    {
        StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (USkeletalMeshComponent* SkeletalMesh = this->FindComponentByClass<USkeletalMeshComponent>())
    {
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 장비 상태가 변경되었을 때 알림 발생
    OnItemStateChanged.Broadcast();
}
