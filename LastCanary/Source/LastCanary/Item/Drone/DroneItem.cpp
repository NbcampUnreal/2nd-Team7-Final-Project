#include "Item/Drone/DroneItem.h"
#include "Character/BasePlayerController.h"

void ADroneItem::UseItem()
{
	Super::UseItem();
	if (GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
		if (PlayerController)
		{
			PlayerController->DroneClass = DroneClass;
		}
	}
}