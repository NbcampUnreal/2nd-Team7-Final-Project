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
                        UE_LOG(LogTemp, Warning, TEXT("Auto-found BaseCharacter target!"));
                        break;
                    }
                }
            }
        }
    }

    if (!Target)
    {
        UE_LOG(LogTemp, Error, TEXT("No suitable BaseCharacter found!"));
    }
}

void ASpawnerTriggerVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (OtherActor == Target)
    {
        Spawning = true;
        UpdateSpawnerState();
        UE_LOG(LogTemp, Warning, TEXT("1111111111111111111111111111111111111111111111111111111111111111111111111111111"));
    }
}

void ASpawnerTriggerVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
    Super::NotifyActorEndOverlap(OtherActor);

    if (OtherActor == Target)
    {
        Spawning = false;
        UpdateSpawnerState();
        UE_LOG(LogTemp, Warning, TEXT("2222222222222222222222222222222222222222222222222222222222222222222222222222222222"));
    }
}

void ASpawnerTriggerVolume::UpdateSpawnerState()
{
    if (!Target)
    {
        UE_LOG(LogTemp, Error, TEXT("Target is NULL!"));
        return;
    }

    UMonsterSpawnComponent* SpawnComponent = Target->FindComponentByClass<UMonsterSpawnComponent>();
    if (!SpawnComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterSpawnComponent not found!"));
        return;
    }

    if (Spawning)
    {
        UE_LOG(LogTemp, Warning, TEXT("33333333333333333333333333333333333333333333333"));
        SpawnComponent->StartSpawning();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("444444444444444444444444444444444444444444444444"));
        SpawnComponent->StopSpawning();
    }
}