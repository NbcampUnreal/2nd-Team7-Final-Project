#include "Framework/Manager/GateCutsceneManager.h"
#include "Character/BaseCharacter.h"
#include "Actor/LCGateActor.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Kismet/GameplayStatics.h"

AGateCutsceneManager::AGateCutsceneManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AGateCutsceneManager::BeginPlay()
{
	Super::BeginPlay();
}

void AGateCutsceneManager::PlayGateCutscene(const TArray<ABaseCharacter*>& InPlayerCharacters)
{
	if (HasAuthority())
	{
		Multicast_PlayCutscene(InPlayerCharacters);
	}
}

void AGateCutsceneManager::Multicast_PlayCutscene_Implementation(const TArray<ABaseCharacter*>& InPlayerCharacters)
{
	PlayerCharacters = InPlayerCharacters;

	for (AActor* Dummy : SpawnedDummies)
	{
		if (IsValid(Dummy)) Dummy->Destroy();
	}
	SpawnedDummies.Empty();
	CachedControllers.Empty();

	if (GetNetMode() != NM_DedicatedServer)
	{
		for (int32 i = 0; i < PlayerCharacters.Num(); ++i)
		{
			ABaseCharacter* RealChar = PlayerCharacters[i];
			if (!IsValid(RealChar)) continue;

			RealChar->SetActorHiddenInGame(true);
			RealChar->SetActorEnableCollision(false);

			if (HasAuthority())
			{
				if (ALCGameMode* GameMode = GetWorld()->GetAuthGameMode<ALCGameMode>())
				{
					GameMode->ForceAllPlayersReady();
				}
			}

			if (APlayerController* PC = Cast<APlayerController>(RealChar->GetController()))
			{
				CachedControllers.Add(PC);
			}

			AActor* Dummy = GetWorld()->SpawnActor<AActor>(DummyCharacterClass, RealChar->GetActorLocation(), RealChar->GetActorRotation());
			if (Dummy)
			{
				SpawnedDummies.Add(Dummy);
				FName TrackTag = FName(FString::Printf(TEXT("Player_%d"), i + 1));
				BindCharacterToTrack(Dummy, TrackTag);
			}
		}

		if (GateSuckInSequence)
		{
			FMovieSceneSequencePlaybackSettings PlaybackSettings;
			SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), GateSuckInSequence, PlaybackSettings, SequenceActor);
			if (SequencePlayer && SequenceActor)
			{
				SequencePlayer->OnFinished.AddDynamic(this, &AGateCutsceneManager::OnCutsceneFinished);
				SequencePlayer->Play();

				AActor* SequenceCam = nullptr;

				if (SequenceActor && SequenceActor->GetSequence())
				{
					UMovieScene* MovieScene = SequenceActor->GetSequence()->GetMovieScene();
					if (MovieScene)
					{
						const TArray<FMovieSceneBinding>& Bindings = MovieScene->GetBindings();
						for (const FMovieSceneBinding& Binding : Bindings)
						{
							if (Binding.GetName().Contains(TEXT("Camera")))
							{
								FMovieSceneSequenceID SequenceID;
								TArrayView<TWeakObjectPtr<UObject>> BoundObjects = SequencePlayer->FindBoundObjects(Binding.GetObjectGuid(), SequenceID);
								for (TWeakObjectPtr<UObject> WeakObj : BoundObjects)
								{
									if (AActor* Actor = Cast<AActor>(WeakObj.Get()))
									{
										SequenceCam = Actor;
										break;
									}
								}
								break;
							}
						}
					}
				}

				if (SequenceCam)
				{
					for (APlayerController* PC : CachedControllers)
					{
						if (IsValid(PC))
						{
							PC->SetViewTargetWithBlend(SequenceCam, 1.0f);
						}
					}
				}
			}
		}
	}
}

void AGateCutsceneManager::BindCharacterToTrack(AActor* DummyActor, const FName& TrackName)
{
	if (SequenceActor && SequencePlayer)
	{
		SequenceActor->SetBindingByTag(TrackName, { DummyActor });
	}
}

void AGateCutsceneManager::OnCutsceneFinished()
{
	// 다시 Ready 상태 초기화
	if (HasAuthority())
	{
		if (ALCGameMode* GameMode = GetWorld()->GetAuthGameMode<ALCGameMode>())
		{
			GameMode->ClearAllPlayersReady();
		}
	}

	for (AActor* Dummy : SpawnedDummies)
	{
		if (IsValid(Dummy)) Dummy->Destroy();
	}
	SpawnedDummies.Empty();

	for (ABaseCharacter* Char : PlayerCharacters)
	{
		if (IsValid(Char))
		{
			Char->SetActorHiddenInGame(false);
			Char->SetActorEnableCollision(true);
		}
	}

	if (IsValid(LinkedGateActor))
	{
		for (APlayerController* PC : CachedControllers)
		{
			if (IsValid(PC))
			{
				LinkedGateActor->IntoGameLevel(PC);
			}
		}
	}
}