#include "Item/Drone/DroneItem.h"
#include "Item/Drone/BaseDrone.h"
#include "Character/BasePlayerController.h"

void ADroneItem::UseItem()
{
    Super::UseItem();

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }

    ABasePlayerController* PlayerController = Cast<ABasePlayerController>(OwnerPawn->GetController());
    if (PlayerController)
    {
        PlayerController->DroneClass = DroneClass;
    }
}