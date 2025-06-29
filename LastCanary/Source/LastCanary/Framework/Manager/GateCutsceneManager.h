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

UENUM(BlueprintType)
enum class ECutsceneType : uint8
{
	GateEntry,
	GateExit
};

UCLASS()
class LASTCANARY_API AGateCutsceneManager : public AActor
{
	GENERATED_BODY()

public:
	AGateCutsceneManager();

	// 기존 함수를 수정하여 타입을 받도록 변경
	void PlayGateCutscene(const TArray<ABaseCharacter*>& InPlayerCharacters, ECutsceneType CutsceneType = ECutsceneType::GateEntry);

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;


	FTimerHandle CutsceneDelayTimer;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	ULevelSequence* GateSuckInSequence;

	// 나가는 시네마틱용 시퀀스
	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	ULevelSequence* GateExitSequence;

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

	// Dummy Character Spawn Points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	TArray<AActor*> ToInGameDummySpawnPoints;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	TArray<AActor*> ToBaseCampDummySpawnPoints;
};