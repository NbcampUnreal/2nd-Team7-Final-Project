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
#include "SaveGame/LCLocalPlayerSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Character/CinematicDummyCharacter.h"
#include "Net/UnrealNetwork.h"


AGateCutsceneManager::AGateCutsceneManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AGateCutsceneManager::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		// 서버에서 레벨 시퀀스 액터를 미리 생성하고 복제 설정
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ReplicatedSequenceActor = GetWorld()->SpawnActor<ALevelSequenceActor>(SpawnParams);
		if (ReplicatedSequenceActor)
		{
			ReplicatedSequenceActor->SetReplicates(true);
			ReplicatedSequenceActor->SetSequence(GateSuckInSequence);
		}
	}
}

void AGateCutsceneManager::PlayGateCutscene(const TArray<ABaseCharacter*>& InPlayerCharacters)
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<ACinematicDummyCharacter*> DummyPlayerCharacters;

	for (int32 i = 0; i < InPlayerCharacters.Num(); ++i)
	{
		ABaseCharacter* Char = InPlayerCharacters[i];
		if (!IsValid(Char))
		{
			continue;
		}

		AActor* Dummy = GetWorld()->SpawnActor<AActor>(DummyCharacterClass, Char->GetActorTransform());
		if (!Dummy)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn dummy for character %s"), *Char->GetName());
			continue;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Spawned dummy %s for character %s"), *Dummy->GetName(), *Char->GetName());
		}

		Dummy->SetReplicates(true);

		ACinematicDummyCharacter* CinematicDummyCharacter = Cast<ACinematicDummyCharacter>(Dummy);
		if (!CinematicDummyCharacter)
		{
			continue;
		}

		CinematicDummyCharacter->ApplyAppearance(Char->GetCustomizationData());
		DummyPlayerCharacters.Add(CinematicDummyCharacter);

		if (ALCPlayerController* PC = Cast<ALCPlayerController>(Char->GetController()))
		{
			PC->SetLinkedGateActor(LinkedGateActor);
			UE_LOG(LogTemp, Log, TEXT("클라이언트에서 시퀀스 실행"));
			PC->Client_HideHUD();
			// 여전히 카메라 제어와 UI는 각 클라이언트에서 진행
			PC->Client_PlayGateCutscene(GateSuckInSequence, CinematicDummyCharacter, Char->GetActorTransform(), i);
		}

		Char->SetActorHiddenInGame(true);
	}

	// ✅ 서버에서 LevelSequenceActor 생성 및 바인딩 처리
	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	ALevelSequenceActor* OutSequenceActor = nullptr;

	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(),
		GateSuckInSequence,
		PlaybackSettings,
		OutSequenceActor
	);

	if (!SequencePlayer || !OutSequenceActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create LevelSequencePlayer"));
		return;
	}

	OutSequenceActor->SetReplicates(true);
	for (int32 i = 0; i < InPlayerCharacters.Num(); ++i)
	{
		ABaseCharacter* Char = InPlayerCharacters[i];
		if (!IsValid(Char))
		{
			continue;
		}

		if (ALCPlayerController* PC = Cast<ALCPlayerController>(Char->GetController()))
		{
			PC->LinkedSequenceActor = OutSequenceActor;
		}

	}

	// ✅ 모든 더미를 시퀀스에 바인딩
	for (int32 i = 0; i < DummyPlayerCharacters.Num(); ++i)
	{
		FName TrackTag = FName(FString::Printf(TEXT("Slot%d"), i + 1));
		OutSequenceActor->SetBindingByTag(TrackTag, { DummyPlayerCharacters[i] });
		UE_LOG(LogTemp, Log, TEXT("바인딩 완료: %s -> %s"), *TrackTag.ToString(), *DummyPlayerCharacters[i]->GetName());
	}

	// ✅ 시퀀스 서버에서 재생
	SequencePlayer->Play();
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


void AGateCutsceneManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGateCutsceneManager, ReplicatedSequenceActor);
}