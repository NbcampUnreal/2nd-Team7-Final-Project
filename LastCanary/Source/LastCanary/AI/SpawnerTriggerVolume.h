#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "SpawnerTriggerVolume.generated.h"

class UMonsterSpawnComponent;
class ABaseCharacter;

UCLASS()
class LASTCANARY_API ASpawnerTriggerVolume : public ATriggerVolume
{
	GENERATED_BODY()
	
public:
	ASpawnerTriggerVolume();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	ABaseCharacter* Target;

	bool Spawning;

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

private:
	void UpdateSpawnerState();
};