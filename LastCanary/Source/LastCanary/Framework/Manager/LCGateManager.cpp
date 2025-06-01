#include "Framework/Manager/LCGateManager.h"
#include "Actor/LCGateActor.h"
#include "Framework/Manager/LCTimeManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h" // TActorIterator

#include "LastCanary.h"

ALCGateManager::ALCGateManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ALCGateManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		GetWorld()->GetTimerManager().SetTimer(
			GateCheckTimerHandle,
			this,
			&ALCGateManager::CheckGateSpawnCondition,
			CheckInterval,
			true
		);
	}
}

void ALCGateManager::CheckGateSpawnCondition()
{
	if (bGateSpawned == true)
	{
		return;
	}

	// TimeManager 가져오기
	for (TActorIterator<ALCTimeManager> It(GetWorld()); It; ++It)
	{
		ALCTimeManager* TimeManager = *It;
		if (TimeManager && TimeManager->ElapsedDay >= RequiredDay)
		{
			TrySpawnGate();
			bGateSpawned = true;
			GetWorld()->GetTimerManager().ClearTimer(GateCheckTimerHandle);
			break;
		}
	}
}

void ALCGateManager::TrySpawnGate()
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (GateClass == nullptr || GateSpawnPoints.Num() == 0)
	{
		LOG_Frame_WARNING(TEXT("[GateManager] No GateClass or SpawnPoints set."));
		return;
	}

	const int32 RandomIndex = FMath::RandRange(0, GateSpawnPoints.Num() - 1);
	AActor* SpawnPoint = GateSpawnPoints[RandomIndex];

	if (SpawnPoint == nullptr)
	{
		return;
	}

	FTransform SpawnTransform = SpawnPoint->GetActorTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ALCGateActor* SpawnedGate = GetWorld()->SpawnActor<ALCGateActor>(GateClass, SpawnTransform, SpawnParams);
	if (SpawnedGate)
	{
		SpawnedGate->TravelType = EGateTravelType::ToBaseCamp;
		SpawnedGate->InteractionPromptText = FText::FromString(TEXT("Press [F] to Escape to Base Camp"));
		LOG_Frame_WARNING(TEXT("[GateManager] Spawned BaseCamp Gate at %s"), *SpawnTransform.GetLocation().ToString());
	}
}
