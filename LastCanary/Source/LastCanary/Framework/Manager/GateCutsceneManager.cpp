#include "Framework/Manager/GateCutsceneManager.h"
#include "Character/BaseCharacter.h"
#include "Actor/LCGateActor.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"
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
	if (HasAuthority() == false)
	{
		return;
	}

	for (int32 i = 0; i < InPlayerCharacters.Num(); ++i)
	{
		ABaseCharacter* Char = InPlayerCharacters[i];
		if (IsValid(Char) == false)
		{
			continue;
		}

		if (ALCPlayerController* PC = Cast<ALCPlayerController>(Char->GetController()))
		{
			// Set linked gate for cutscene -> game transition
			PC->SetLinkedGateActor(LinkedGateActor);

			PC->Client_HideHUD();
			PC->Client_PlayGateCutscene(GateSuckInSequence, DummyCharacterClass, Char->GetActorTransform(), i);
		}

		Char->SetActorHiddenInGame(true);
	}
}

//void AGateCutsceneManager::Client_HideHUD_Implementation()
//{
//	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
//	{
//		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
//		{
//			UIManager->HideInGameHUD();
//			UIManager->HideSpectatorWidget();
//		}
//	}
//}
//
//void AGateCutsceneManager::Client_PlayGateCutscene_Implementation(const FTransform& DummyTransform, int32 PlayerIndex)
//{
//	if (!GateSuckInSequence || IsNetMode(NM_DedicatedServer)) return;
//
//	FMovieSceneSequencePlaybackSettings Settings;
//	ALevelSequenceActor* OutSequenceActor = nullptr;
//	ULevelSequencePlayer* LocalPlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), GateSuckInSequence, Settings, OutSequenceActor);
//
//	if (!LocalPlayer || !OutSequenceActor) return;
//
//	// 자신만 Dummy 생성
//	AActor* Dummy = GetWorld()->SpawnActor<AActor>(DummyCharacterClass, DummyTransform);
//	if (!IsValid(Dummy)) return;
//
//	FName TrackTag = FName(FString::Printf(TEXT("Slot%d"), PlayerIndex + 1));
//	OutSequenceActor->SetBindingByTag(TrackTag, { Dummy });
//
//	SpawnedDummies = { Dummy };
//	SequencePlayer = LocalPlayer;
//	SequenceActor = OutSequenceActor;
//
//	SequencePlayer->OnFinished.AddDynamic(this, &AGateCutsceneManager::OnCutsceneFinished);
//	SequencePlayer->Play();
//}
//
//void AGateCutsceneManager::OnCutsceneFinished()
//{
//	for (AActor* Dummy : SpawnedDummies)
//	{
//		if (IsValid(Dummy))
//		{
//			Dummy->Destroy();
//		}
//	}
//	SpawnedDummies.Empty();
//
//	for (ABaseCharacter* Char : PlayerCharacters)
//	{
//		if (IsValid(Char))
//		{
//			Char->SetActorHiddenInGame(false);
//		}
//	}
//
//	// 레벨 이동 처리
//	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
//	{
//		if (ALCPlayerController* LCPC = Cast<ALCPlayerController>(PC))
//		{
//			if (ALCGateActor* Gate = LCPC->GetLinkedGateActor())
//			{
//				Gate->IntoGameLevel(LCPC);
//			}
//		}
//	}
//
//	// HUD 복원
//	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
//	{
//		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
//		{
//			UIManager->ShowInGameHUD();
//		}
//	}
//}