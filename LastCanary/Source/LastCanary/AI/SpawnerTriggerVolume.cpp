#include "AI/SpawnerTriggerVolume.h"
#include "AI/MonsterSpawnComponent.h"
#include "Character/BaseCharacter.h"

ASpawnerTriggerVolume::ASpawnerTriggerVolume()
{
    PrimaryActorTick.bCanEverTick = false;

    Spawning = false;
}

void ASpawnerTriggerVolume::BeginPlay()
{
    Super::BeginPlay();
    SetActorEnableCollision(true);

    if (!Target)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                ABaseCharacter* BaseChar = Cast<ABaseCharacter>(Actor);
                if (BaseChar)
                {
                    if (BaseChar->FindComponentByClass<UMonsterSpawnComponent>())
                    {
                        Target = BaseChar;
                        break;
                    }
                }
            }
        }
    }
}

void ASpawnerTriggerVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (OtherActor == Target)
    {
        Spawning = true;
        UpdateSpawnerState();
    }
}

void ASpawnerTriggerVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
    Super::NotifyActorEndOverlap(OtherActor);

    if (OtherActor == Target)
    {
        Spawning = false;
        UpdateSpawnerState();
    }
}

void ASpawnerTriggerVolume::UpdateSpawnerState()
{
    if (!Target)
    {
        return;
    }

    UMonsterSpawnComponent* SpawnComponent = Target->FindComponentByClass<UMonsterSpawnComponent>();
    if (!SpawnComponent)
    {
        return;
    }

    if (Spawning)
    {
        SpawnComponent->StartSpawning();
    }
    else
    {
        SpawnComponent->StopSpawning();
    }
}