#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GateCutsceneManager.generated.h"

class ULevelSequence;
class ALevelSequenceActor;
class ULevelSequencePlayer;
class ACinematicDummyCharacter;
class ALCGateActor;
class APlayerController;
class ABaseCharacter;

UCLASS()
class LASTCANARY_API AGateCutsceneManager : public AActor
{
	GENERATED_BODY()

public:
	AGateCutsceneManager();

	UFUNCTION(BlueprintCallable)
	void PlayGateCutscene(const TArray<ABaseCharacter*>& InPlayerCharacters);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayCutscene(const TArray<ABaseCharacter*>& InPlayerCharacters);
	void Multicast_PlayCutscene_Implementation(const TArray<ABaseCharacter*>& InPlayerCharacters);

	UFUNCTION()
	void OnCutsceneFinished();

	UPROPERTY(EditInstanceOnly, Category = "Cutscene")
	ALCGateActor* LinkedGateActor;

protected:
	virtual void BeginPlay() override;

	void BindCharacterToTrack(AActor* DummyActor, const FName& TrackName);

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TSubclassOf<ABaseCharacter> DummyCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	ULevelSequence* GateSuckInSequence;

	UPROPERTY()
	ALevelSequenceActor* SequenceActor;

	UPROPERTY()
	ULevelSequencePlayer* SequencePlayer;

	UPROPERTY()
	TArray<AActor*> SpawnedDummies;

	UPROPERTY()
	TArray<ABaseCharacter*> PlayerCharacters;

	UPROPERTY()
	TArray<APlayerController*> CachedControllers;
};