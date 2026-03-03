#include "Door_Warder.h"
#include "../../AI/NormalMonster/WarderBaseMonster.h" 
#include "Perception/AISense_Hearing.h"

ADoor_Warder::ADoor_Warder()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADoor_Warder::Interact_Implementation(APlayerController* Interactor)
{
	if (WarderMonster)
	{
		float TargetYaw = bIsOpen ? -90.f : 90.f;
		AddActorLocalRotation(FRotator(0.f, TargetYaw, 0.f));
		bIsOpen = !bIsOpen;

		UAISense_Hearing::ReportNoiseEvent(
			GetWorld(),
			GetActorLocation(),
			3.0f,
			this,
			20000.f,
			"WarderSearch"
		);

		UE_LOG(LogTemp, Warning, TEXT("Door: Interact"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Door: Monster Empty"));
	}
}