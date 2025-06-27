#include "Actor/LCBossSpawner.h"

ALCBossSpawner::ALCBossSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ALCBossSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALCBossSpawner::SpawnBoss(TSubclassOf<AActor> BossClass)
{
	const int32 RandomIndex = FMath::RandRange(0, BossSpawnPoints.Num() - 1);
	AActor* SpawnPoint = BossSpawnPoints[RandomIndex];

	if (SpawnPoint == nullptr)
	{
		return;
	}

	FTransform SpawnTransform = SpawnPoint->GetActorTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedBoss = GetWorld()->SpawnActor<AActor>(BossClass, SpawnTransform, SpawnParams);
	if (SpawnedBoss)
	{
		UE_LOG(LogTemp, Log, TEXT("Boss spawned at %s"), *SpawnTransform.GetLocation().ToString());
	}

}

