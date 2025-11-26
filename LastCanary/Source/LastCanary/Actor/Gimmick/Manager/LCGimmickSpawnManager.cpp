#include "Actor/Gimmick/Manager/LCGimmickSpawnManager.h"
#include "Actor/Gimmick/LCGimmickSpawner.h"
#include "Interface/LCGimmickSpawnInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "LastCanary.h"

ALCGimmickSpawnManager::ALCGimmickSpawnManager()
	: SpawnDelay(0.1f)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ALCGimmickSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (SpawnDelay <= 0.f)
	{
		ExecuteSpawn();
	}
	else
	{
		GetWorldTimerManager().SetTimer(SpawnTimer, this, &ALCGimmickSpawnManager::ExecuteSpawn, SpawnDelay, false);
	}
}

bool ALCGimmickSpawnManager::CanSpawnClass(TSubclassOf<AActor> Class)
{
	if (!IsValid(Class)) return false;

	const FGlobalSpawnQuota* Found = GlobalQuotas.FindByPredicate(
		[&](const FGlobalSpawnQuota& Q) { return Q.Class == Class; });

	if (!Found) return true;

	const int32* Current = SpawnedCount.Find(Class);
	const int32 Curr = Current ? *Current : 0;

	return Curr < Found->MaxCount;
}

void ALCGimmickSpawnManager::ExecuteSpawn()
{
	TArray<AActor*> FoundSpawners;
	UGameplayStatics::GetAllActorsOfClass(this, ALCGimmickSpawner::StaticClass(), FoundSpawners);

	TMap<int32, TArray<ALCGimmickSpawner*>> Groups;
	for (AActor* A : FoundSpawners)
	{
		if (ALCGimmickSpawner* S = Cast<ALCGimmickSpawner>(A))
		{
			Groups.FindOrAdd(S->GroupId).Add(S);
		}
	}

	for (const TPair<int32, TArray<ALCGimmickSpawner*>>& Pair : Groups)
	{
		for (ALCGimmickSpawner* Spawner : Pair.Value)
		{
			if (!IsValid(Spawner)) continue;

			TSubclassOf<AActor> Picked = nullptr;
			{
				const int32 MaxTry = 8;
				int32 Try = 0;
				while (Try++ < MaxTry)
				{
					TSubclassOf<AActor> Temp = Spawner->PickClassByWeight();
					if (!IsValid(Temp)) break;
					if (CanSpawnClass(Temp))
					{
						Picked = Temp;
						break;
					}
				}
			}

			if (!IsValid(Picked))
			{
				LOG_Art_WARNING(TEXT("[SpawnManager] No spawnable class for %s (quota exhausted or no candidates)"), *GetNameSafe(Spawner));
				continue;
			}

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FTransform Xform = Spawner->bUseSpawnerTransform ? Spawner->GetActorTransform() : FTransform::Identity;
			AActor* NewActor = GetWorld()->SpawnActor<AActor>(Picked, Xform, Params);
			if (!IsValid(NewActor))
			{
				LOG_Art_ERROR(TEXT("[SpawnManager] Spawn failed: %s"), *Picked->GetName());
				continue;
			}

			Spawner->SpawnedActor = NewActor;

			int32& Cnt = SpawnedCount.FindOrAdd(Picked);
			++Cnt;

			LOG_Art(Log, TEXT("[SpawnManager] Spawned %s at %s (Group:%d, GimmickRole:%d)"),
				*GetNameSafe(NewActor), *NewActor->GetActorLocation().ToString(), Spawner->GroupId, (int32)Spawner->GimmickRole);

			LinkActors(Spawner, NewActor, Groups);

			Multicast_OnSpawned(NewActor, Spawner);
		}
	}
}

void ALCGimmickSpawnManager::LinkActors(ALCGimmickSpawner* Spawner, AActor* Spawned, const TMap<int32, TArray<ALCGimmickSpawner*>>& Groups)
{
	if (!IsValid(Spawner) || !IsValid(Spawned)) return;

	if (Spawner->ExternalLinkedActor.IsValid())
	{
		AActor* External = Spawner->ExternalLinkedActor.Get();
		if (IsValid(External))
		{
			switch (Spawner->GimmickRole)
			{
			case ESpawnerRole::Trigger:
				TrySetLinkedTarget(Spawned, External);
				TrySetLinkedTrigger(External, Spawned);
				break;
			case ESpawnerRole::Target:
				TrySetLinkedTrigger(Spawned, External);
				TrySetLinkedTarget(External, Spawned);
				break;
			case ESpawnerRole::SelfContained:
				TrySetLinkedTarget(Spawned, External);
				TrySetLinkedTrigger(Spawned, External);
				break;
			}
			return;
		}
	}

	const TArray<ALCGimmickSpawner*>* SameGroup = Groups.Find(Spawner->GroupId);
	if (!SameGroup)
	{
		if (Spawner->GimmickRole == ESpawnerRole::SelfContained)
		{
			TrySetLinkedTarget(Spawned, Spawned);
			TrySetLinkedTrigger(Spawned, Spawned);
		}
		return;
	}

	AActor* TriggerActor = nullptr;
	AActor* TargetActor = nullptr;

	for (ALCGimmickSpawner* S : *SameGroup)
	{
		if (!IsValid(S) || !IsValid(S->SpawnedActor)) continue;
		if (S->GimmickRole == ESpawnerRole::Trigger)  TriggerActor = S->SpawnedActor;
		if (S->GimmickRole == ESpawnerRole::Target)   TargetActor = S->SpawnedActor;
	}

	if (Spawner->GimmickRole == ESpawnerRole::SelfContained)
	{
		TrySetLinkedTarget(Spawned, Spawned);
		TrySetLinkedTrigger(Spawned, Spawned);
	}

	if (TriggerActor && TargetActor)
	{
		TrySetLinkedTarget(TriggerActor, TargetActor);
		TrySetLinkedTrigger(TargetActor, TriggerActor);
	}
}

void ALCGimmickSpawnManager::TrySetLinkedTarget(AActor* OnActor, AActor* Target)
{
	if (!IsValid(OnActor)) return;
	if (OnActor->GetClass()->ImplementsInterface(ULCGimmickSpawnInterface::StaticClass()))
	{
		ILCGimmickSpawnInterface::Execute_SetLinkedTarget(OnActor, Target);
	}
}

void ALCGimmickSpawnManager::TrySetLinkedTrigger(AActor* OnActor, AActor* Trigger)
{
	if (!IsValid(OnActor)) return;
	if (OnActor->GetClass()->ImplementsInterface(ULCGimmickSpawnInterface::StaticClass()))
	{
		ILCGimmickSpawnInterface::Execute_SetLinkedTrigger(OnActor, Trigger);
	}
}

void ALCGimmickSpawnManager::Multicast_OnSpawned_Implementation(AActor* Spawned, ALCGimmickSpawner* Spawner)
{
	if (!IsValid(Spawner) || !IsValid(Spawned)) return;
	LOG_Art(Log, TEXT("[SpawnManager][MC] %s spawned by %s (Group:%d, GimmickRole:%d)"),
		*GetNameSafe(Spawned), *GetNameSafe(Spawner), Spawner->GroupId, (int32)Spawner->GimmickRole);
}