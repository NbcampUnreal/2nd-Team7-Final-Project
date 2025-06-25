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

	void PlayGateCutscene(const TArray<ABaseCharacter*>& InPlayerCharacters);

protected:
	virtual void BeginPlay() override;

	//UFUNCTION(Client, Reliable)
	//void Client_PlayGateCutscene(const FTransform& DummyTransform, int32 PlayerIndex);
	//void Client_PlayGateCutscene_Implementation(const FTransform& DummyTransform, int32 PlayerIndex);


	//UFUNCTION(Client, Reliable)
	//void Client_HideHUD();
	//void Client_HideHUD_Implementation();

	//UFUNCTION()
	//void OnCutsceneFinished();
	UPROPERTY(Replicated)
	class ALevelSequenceActor* ReplicatedSequenceActor;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;


	FTimerHandle CutsceneDelayTimer;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	ULevelSequence* GateSuckInSequence;

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TSubclassOf<AActor> DummyCharacterClass;
//
//	UPROPERTY()
//	TArray<ABaseCharacter*> PlayerCharacters;
//
//	UPROPERTY()
//	TArray<AActor*> SpawnedDummies;
//
//	UPROPERTY()
//	ULevelSequencePlayer* SequencePlayer;
//
//	UPROPERTY()
//	ALevelSequenceActor* SequenceActor;
//
public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Cutscene")
	TObjectPtr<ALCGateActor> LinkedGateActor;
};