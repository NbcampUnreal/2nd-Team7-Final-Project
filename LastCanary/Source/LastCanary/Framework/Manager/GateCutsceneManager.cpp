#include "Framework/Manager/GateCutsceneManager.h"
#include "Character/BaseCharacter.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "Character/CinematicDummyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"


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

void AGateCutsceneManager::PlayGateCutscene(const TArray<ABaseCharacter*>& InPlayerCharacters, ECutsceneType CutsceneType)
{
    if (!HasAuthority())
    {
        return;
    }

    // 시퀀스 선택
    ULevelSequence* SelectedSequence = nullptr;
    TArray<AActor*> SpawnPoints;
    switch (CutsceneType)
    {
    case ECutsceneType::GateEntry:
        SelectedSequence = GateSuckInSequence;
        SpawnPoints = ToInGameDummySpawnPoints;
        break;
    case ECutsceneType::GateExit:
        SelectedSequence = GateExitSequence;
        SpawnPoints = ToBaseCampDummySpawnPoints;
        break;
    }

    if (!SelectedSequence)
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

        FTransform SpawnPoint = SpawnPoints.IsValidIndex(i)
            ? SpawnPoints[i]->GetActorTransform()
            : Char->GetActorTransform();

        // 미리 정해둘 이름
        FName DummyName = FName(*FString::Printf(TEXT("CinematicDummy_%d"), i));

        // 동일 이름의 더미가 이미 있는지 검사
        ACinematicDummyCharacter* ExistingDummy = nullptr;
        for (TActorIterator<ACinematicDummyCharacter> It(GetWorld()); It; ++It)
        {
            if (It->GetFName() == DummyName)
            {
                ExistingDummy = *It;
                break;
            }
        }

        if (ExistingDummy)
        {
            DummyPlayerCharacters.Add(ExistingDummy);
            ExistingDummy->ApplyAppearance(Char->GetCustomizationData());
            ExistingDummy->CutsceneIndex = i;
            Char->SetActorHiddenInGame(true);
            continue;
        }

        // 없다면 새로 스폰
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.Name = DummyName;

        ACinematicDummyCharacter* Dummy = GetWorld()->SpawnActor<ACinematicDummyCharacter>(
            DummyCharacterClass, SpawnPoint, SpawnParams);

        if (!Dummy)
        {
            continue;
        }

        Dummy->SetReplicates(true);
        Dummy->ApplyAppearance(Char->GetCustomizationData());
        Dummy->CutsceneIndex = i;
        DummyPlayerCharacters.Add(Dummy);
        Char->SetActorHiddenInGame(true);
    }

    // LevelSequenceActor 생성 (리플리케이트)
    FMovieSceneSequencePlaybackSettings PlaybackSettings;
    ALevelSequenceActor* OutSequenceActor = nullptr;

    ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
        GetWorld(),
        SelectedSequence,
        PlaybackSettings,
        OutSequenceActor
    );

    if (!SequencePlayer || !OutSequenceActor)
    {
        return;
    }

    OutSequenceActor->SetReplicates(true);
    CachedPlayerCharacters = InPlayerCharacters;          // 멤버 변수에 저장
    CachedSequenceActor = OutSequenceActor;
    CachedCutsceneType = CutsceneType;

    GetWorld()->GetTimerManager().SetTimer(
        CutsceneRPC_TimerHandle,
        this,
        &AGateCutsceneManager::DelayedSendCutsceneRPC,
        0.5f,     // 딜레이 시간
        false
    );

    // 서버에서 시퀀스 재생
    SequencePlayer->Play();
}

void AGateCutsceneManager::DelayedSendCutsceneRPC()
{
    for (int32 i = 0; i < CachedPlayerCharacters.Num(); ++i)
    {
        ABaseCharacter* Char = CachedPlayerCharacters[i];
        if (!IsValid(Char))
        {
            continue;
        }

        if (ALCPlayerController* PC = Cast<ALCPlayerController>(Char->GetController()))
        {
            PC->LinkedSequenceActor = CachedSequenceActor;
            PC->Client_PlayGateCutscene(CachedSequenceActor, CachedPlayerCharacters.Num(), CachedCutsceneType);
            PC->Client_HideHUD();
            PC->SetLinkedGateActor(LinkedGateActor);
        }
    }
}

void AGateCutsceneManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AGateCutsceneManager, ReplicatedSequenceActor);
}

//void AGateCutsceneManager::PlayGateCutscene(const TArray<ABaseCharacter*>& InPlayerCharacters)
//{
//	if (!HasAuthority())
//	{
//		return;
//	}
//
//	// 나가는 시네마틱이 설정되지 않은 경우 바로 리턴
//	if (!GateExitSequence)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("GateExitSequence is not set!"));
//		return;
//	}
//
//	TArray<ACinematicDummyCharacter*> DummyPlayerCharacters;
//
//	for (int32 i = 0; i < InPlayerCharacters.Num(); ++i)
//	{
//		ABaseCharacter* Char = InPlayerCharacters[i];
//		if (!IsValid(Char))
//		{
//			continue;
//		}
//
//		AActor* Dummy = GetWorld()->SpawnActor<AActor>(DummyCharacterClass, Char->GetActorTransform());
//		if (!Dummy)
//		{
//			UE_LOG(LogTemp, Error, TEXT("Failed to spawn dummy for character %s"), *Char->GetName());
//			continue;
//		}
//		else
//		{
//			UE_LOG(LogTemp, Log, TEXT("Spawned dummy %s for character %s"), *Dummy->GetName(), *Char->GetName());
//		}
//
//		Dummy->SetReplicates(true);
//
//		ACinematicDummyCharacter* CinematicDummyCharacter = Cast<ACinematicDummyCharacter>(Dummy);
//		if (!CinematicDummyCharacter)
//		{
//			continue;
//		}
//
//		CinematicDummyCharacter->ApplyAppearance(Char->GetCustomizationData());
//		DummyPlayerCharacters.Add(CinematicDummyCharacter);
//
//		if (ALCPlayerController* PC = Cast<ALCPlayerController>(Char->GetController()))
//		{
//			PC->SetLinkedGateActor(LinkedGateActor);
//			UE_LOG(LogTemp, Log, TEXT("클라이언트에서 시퀀스 실행"));
//			PC->Client_HideHUD();
//			// 여전히 카메라 제어와 UI는 각 클라이언트에서 진행
//			PC->Client_PlayGateCutscene(GateSuckInSequence, CinematicDummyCharacter, Char->GetActorTransform(), i);
//		}
//
//		Char->SetActorHiddenInGame(true);
//	}
//
//	// ✅ 서버에서 LevelSequenceActor 생성 및 바인딩 처리
//	FMovieSceneSequencePlaybackSettings PlaybackSettings;
//	ALevelSequenceActor* OutSequenceActor = nullptr;
//
//	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
//		GetWorld(),
//		GateSuckInSequence,
//		GateExitSequence,
//		PlaybackSettings,
//		OutSequenceActor
//	);
//
//	if (!SequencePlayer || !OutSequenceActor)
//	{
//		UE_LOG(LogTemp, Error, TEXT("Failed to create LevelSequencePlayer"));
//		return;
//	}
//
//	OutSequenceActor->SetReplicates(true);
//	for (int32 i = 0; i < InPlayerCharacters.Num(); ++i)
//	{
//		ABaseCharacter* Char = InPlayerCharacters[i];
//		if (!IsValid(Char))
//		{
//			continue;
//		}
//
//		if (ALCPlayerController* PC = Cast<ALCPlayerController>(Char->GetController()))
//		{
//			PC->LinkedSequenceActor = OutSequenceActor;
//		}
//
//	}
//
//	// ✅ 모든 더미를 시퀀스에 바인딩
//	for (int32 i = 0; i < DummyPlayerCharacters.Num(); ++i)
//	{
//		FName TrackTag = FName(FString::Printf(TEXT("Slot%d"), i + 1));
//		OutSequenceActor->SetBindingByTag(TrackTag, { DummyPlayerCharacters[i] });
//		UE_LOG(LogTemp, Log, TEXT("바인딩 완료: %s -> %s"), *TrackTag.ToString(), *DummyPlayerCharacters[i]->GetName());
//	}
//
//	// ✅ 시퀀스 서버에서 재생
//	SequencePlayer->Play();
//}


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
