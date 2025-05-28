#include "Actor/Gimmick/Effect/LCGimmickDoor.h"
#include "Components/StaticMeshComponent.h"

#include "LastCanary.h"

ALCGimmickDoor::ALCGimmickDoor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    RootComponent = DoorMesh;
}

void ALCGimmickDoor::TriggerEffect_Implementation()
{
    if (bIsOpen)
    {
        return;
    }
    bIsOpen = true;

    if (bSlideInstead)
    {
        FVector NewLocation = GetActorLocation() + SlideOffset;
        SetActorLocation(NewLocation);
    }
    else
    {
        FRotator NewRotation = GetActorRotation() + OpenRotation;
        SetActorRotation(NewRotation);
    }

    LOG_Frame_WARNING( TEXT("[GimmickDoor] Door opened."));
}
