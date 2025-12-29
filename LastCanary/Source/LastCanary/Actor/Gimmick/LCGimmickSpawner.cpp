#include "Actor/Gimmick/LCGimmickSpawner.h"
#include "LastCanary.h"

ALCGimmickSpawner::ALCGimmickSpawner()
	: GroupId(0)
	, GimmickRole(ESpawnerRole::SelfContained)
	, SpawnedActor(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ALCGimmickSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

TSubclassOf<AActor> ALCGimmickSpawner::PickClassByWeight() const
{
	float Total = 0.f;
	for (const FSpawnCandidate& C : Candidates)
	{
		if (IsValid(C.Class) && C.Weight > 0.f)
		{
			Total += C.Weight;
		}
	}
	if (Total <= 0.f)
	{
		LOG_Art_WARNING(TEXT("[Spawner] No valid candidates: %s"), *GetName());
		return nullptr;
	}

	const float R = FMath::FRandRange(0.f, Total);
	float Acc = 0.f;
	for (const FSpawnCandidate& C : Candidates)
	{
		if (!IsValid(C.Class) || C.Weight <= 0.f) continue;
		Acc += C.Weight;
		if (R <= Acc)
		{
			return C.Class;
		}
	}
	return Candidates.Num() > 0 ? Candidates[0].Class : nullptr;
}