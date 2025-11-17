#include "Art/Ruins/Maze/RuinsMazeWall.h"
#include "Components/StaticMeshComponent.h"

ARuinsMazeWall::ARuinsMazeWall()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);

    WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
    SetRootComponent(WallMesh);

    WallMesh->SetCollisionProfileName(TEXT("BlockAll"));
    WallMesh->SetGenerateOverlapEvents(false);

    // 최적화
    WallMesh->LDMaxDrawDistance = 10000.f;
    WallMesh->bAllowCullDistanceVolume = true;
}

void ARuinsMazeWall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


