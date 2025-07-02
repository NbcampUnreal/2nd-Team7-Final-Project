#include "Framework/PlayerController/LCPlayerController.h"

#include "Framework/GameMode/LCGameMode.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "Character/BasePlayerState.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"

#include "Framework/Manager/LCCheatManager.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIElement/RoomWidget.h"

#include "Net/UnrealNetwork.h"

#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "LevelSequence.h"
#include "Actor/LCGateActor.h"
#include "CineCameraActor.h"
#include "MovieSceneSequencePlayer.h"
#include "Character/CinematicDummyCharacter.h"
#include "EngineUtils.h"

ALCPlayerController::ALCPlayerController()
{
    CheatClass = ULCCheatManager::StaticClass();
}

void ALCPlayerController::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();

}

void ALCPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
    {
        if (ULCUIManager* UIManager = Subsystem->GetUIManager())
        {
            LCUIManager = UIManager;
            LCUIManager->InitUIManager(this);
            LCUIManager->SetPlayerController(this);
        }
    }

    // 복구 타이머
    FTimerHandle InventoryRestoreHandle;
    GetWorld()->GetTimerManager().SetTimer(InventoryRestoreHandle, this, &ALCPlayerController::TryRestoreInventory, 0.3f, false);

    if (CheatManager == nullptr)
    {
        CheatManager = NewObject<ULCCheatManager>(this, CheatClass);
        CheatManager->InitCheatManager();
    }

    LOG_Frame_WARNING(TEXT("PostSeamlessTravel: Ensuring CheatManager is ready"));

    LOG_Frame_WARNING(TEXT("PostSeamlessTravel: %s 호출 - IsLocalController: %d"), *GetName(), IsLocalController());
    GetWorldTimerManager().SetTimerForNextTick(this, &ALCPlayerController::DelayedPostTravelSetup);
}

void ALCPlayerController::TryRestoreInventory()
{
    if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
    {
        if (ABaseCharacter* Char = Cast<ABaseCharacter>(GetPawn()))
        {
            if (UToolbarInventoryComponent* Toolbar = Char->GetToolbarInventoryComponent())
            {
                Toolbar->SetInventoryFromItemIDs(PS->AquiredItemIDs);
                LOG_Frame_WARNING(TEXT("[TryRestoreInventory] 복원 시도 완료. 아이템 수: %d"), PS->AquiredItemIDs.Num());
            }
        }
    }
}

void ALCPlayerController::DelayedPostTravelSetup()
{
    LOG_Frame_WARNING(TEXT("PostSeamlessTravel(Delayed): %s - 여전히 IsLocalController: %d"), *GetName(), IsLocalController());

    if (IsLocalController())
    {
        if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
        {
            if (ULCUIManager* UIManager = Subsystem->GetUIManager())
            {
                UIManager->SetPlayerController(this);
                LOG_Frame_WARNING(TEXT("DelayedPostTravelSetup: UIManager에 컨트롤러 연결 완료"));
            }
        }
    }
}

void ALCPlayerController::Server_SetPlayerInfo_Implementation(const FSessionPlayerInfo& PlayerInfo)
{
    if (UWorld* World = GetWorld())
    {
        if (ALCGameMode* LCGM = Cast<ALCGameMode>(World->GetAuthGameMode()))
        {
            LCGM->SetPlayerInfo(PlayerInfo);
        }
    }
}

void ALCPlayerController::Client_UpdatePlayerList_Implementation(const TArray<FSessionPlayerInfo>& PlayerInfos)
{
    UpdatePlayerList(PlayerInfos);

}

void ALCPlayerController::UpdatePlayerList(const TArray<FSessionPlayerInfo>& PlayerInfos)
{
    if (IsValid(LCUIManager))
    {
        LOG_Frame_WARNING(TEXT("LCUIManager Is Not Null!"));
        URoomWidget* RoomWidget = LCUIManager->GetRoomWidgetInstance();
        RoomWidget->UpdatePlayerLists(PlayerInfos);

        GetWorld()->GetTimerManager().ClearTimer(UpdatePlayerListTimerHandle);
    }
    else
    {
        TWeakObjectPtr<ALCPlayerController> WeakPtr(this);
        TArray<FSessionPlayerInfo> InfosCopy = PlayerInfos;

        GetWorld()->GetTimerManager().SetTimer
        (
            UpdatePlayerListTimerHandle,
            [WeakPtr, InfosCopy]()
            {
                if (WeakPtr.IsValid())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Update Lobby UI!!"));
                    WeakPtr->UpdatePlayerList(InfosCopy);
                }
            },
            RePeatRate,
            false
        );
    }
}

void ALCPlayerController::ToggleShowRoomWidget()
{
    Super::ToggleShowRoomWidget();
    bIsShowRoomUI = !bIsShowRoomUI;
    LOG_Frame_WARNING(TEXT("ToggleShowRoomWidget: %s"), bIsShowRoomUI ? TEXT("Show") : TEXT("Hide"));

    if (bIsShowRoomUI)
    {
        LCUIManager->ShowRoomWidget();
    }
    else
    {
        LCUIManager->HideRoomWidget();
    }

    bShowMouseCursor = bIsShowRoomUI;
}

void ALCPlayerController::Client_ShowLoading_Implementation()
{
    if (LCUIManager)
    {
        LCUIManager->ShowLoadingLevel();
    }
}


void ALCPlayerController::Client_HideLoading_Implementation()
{
    if (LCUIManager)
    {
        LCUIManager->HideLoadingLevel();
    }
}

void ALCPlayerController::Client_ReceiveMessageFromGM_Implementation(const FString& Message)
{
    LOG_Server_WARNING(TEXT("%s"), *Message);
	if (LCUIManager)
	{
        LCUIManager->AddServerMessage(Message);// ShowPopupNotice(FText::FromString(Message));
	}

}

void ALCPlayerController::ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason)
{
    Super::ClientReturnToMainMenuWithTextReason_Implementation(ReturnReason);

}

void ALCPlayerController::ClientWasKicked_Implementation(const FText& KickReason)
{
    LOG_Server_ERROR(TEXT("Kicked By Server!!"));

    if (LCUIManager)
    {
		LCUIManager->SetSessionErrorState(KickReason);
    }
}

void ALCPlayerController::StartGame(FString SoftPath)
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("Try Start Game!!"));
    if (UWorld* World = GetWorld())
    {
        if (ALCGameMode* LCGM = Cast<ALCGameMode>(World->GetAuthGameMode()))
        {
            LCGM->TravelMapBySoftPath(SoftPath);
        }
    }
}

void ALCPlayerController::Client_HideHUD_Implementation()
{
    if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
    {
        if (ULCUIManager* UIManager = Subsystem->GetUIManager())
        {
            UIManager->HideInGameHUD();
            UIManager->HideSpectatorWidget();
        }
    }
}

void ALCPlayerController::Client_PlayGateCutscene_Implementation(
    ALevelSequenceActor* SequenceActor,
    int32 TotalPlayers,
    ECutsceneType CutsceneType)
{
    if (!IsValid(SequenceActor) || !IsValid(SequenceActor->SequencePlayer))
    {
        return;
    }
    
    CurrentCutsceneType = CutsceneType;

    // 월드에서 모든 더미를 이름으로 찾아 바인딩
    for (int32 i = 0; i < TotalPlayers; ++i)
    {
        ACinematicDummyCharacter* FoundDummy = nullptr;
        for (TActorIterator<ACinematicDummyCharacter> It(GetWorld()); It; ++It)
        {            
            if (It->CutsceneIndex == i)
            {
                FoundDummy = *It;
                break;
            }
        }

        if (IsValid(FoundDummy))
        {
            FName TrackTag = FName(FString::Printf(TEXT("Slot%d"), i + 1));
            SequenceActor->SetBindingByTag(TrackTag, { FoundDummy });            
        }
    }

    // 시퀀스 재생
    SequenceActor->SequencePlayer->Play();

    // 카메라 전환 & UI 숨기기
    SetCameraFromSequence(SequenceActor);
    HideUIForCutscene();

    // OnFinished 콜백 등록 (필요하면)
    SequenceActor->SequencePlayer->OnFinished.AddUniqueDynamic(this, &ALCPlayerController::OnCutsceneFinished);
}

// 카메라 설정을 별도 함수로 분리
void ALCPlayerController::SetCameraFromSequence(ALevelSequenceActor* SequenceActor)
{
    if (!SequenceActor) return;

    FName CameraTag = TEXT("Camera");
    TArray<FMovieSceneObjectBindingID> Bindings = SequenceActor->GetSequence()->FindBindingsByTag(CameraTag);

    if (Bindings.Num() > 0)
    {
        FMovieSceneObjectBindingID BindingID = Bindings[0];
        TArray<UObject*> BoundObjects = SequenceActor->GetSequencePlayer()->GetBoundObjects(BindingID);
        for (UObject* Obj : BoundObjects)
        {
            if (ACameraActor* CameraActor = Cast<ACameraActor>(Obj))
            {
                if (IsLocalController())
                {
                    SetViewTargetWithBlend(CameraActor, 0.0f);
                }
                break;
            }
        }
    }
}

// UI 숨기기를 별도 함수로 분리
void ALCPlayerController::HideUIForCutscene()
{
    if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
    {
        if (ULCUIManager* UIManager = Subsystem->GetUIManager())
        {
            UIManager->HideInGameHUD();
            UIManager->HideSpectatorWidget();
        }
    }
}

// UI 복원을 별도 함수로 분리
void ALCPlayerController::ShowUIAfterCutscene()
{
    if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
    {
        if (ULCUIManager* UIManager = Subsystem->GetUIManager())
        {
            UIManager->ShowInGameHUD();
        }
    }
}

// 통합된 컷신 종료 처리
void ALCPlayerController::OnCutsceneFinished()
{
    // 컷신 타입에 따라 다른 처리
    switch (CurrentCutsceneType)
    {
    case ECutsceneType::GateEntry:
        // 두번 호출돼서 호스트만 하도록 수정
        if (HasAuthority())
        {
            LinkedGateActor->IntoGameLevel(this);
        }
        // 게임 레벨로 이동
        //Server_RequestIntoGameLevel();
        break;
    case ECutsceneType::GateExit:
        // 베이스로 돌아가기 -> 베이스캠프에서 실행하기때문에 지움
        Server_RequestReturnToBase();
        ShowUIAfterCutscene(); // 나가는 경우에만 UI 복원
        //ACharacter* Char = GetCharacter();
        //Char->SetActorHiddenInGame(false);
        break;
    }
}

void ALCPlayerController::Server_RequestIntoGameLevel_Implementation()
{
    if (IsValid(LinkedGateActor))
    {
        LinkedGateActor->IntoGameLevel(this);
    }
}

void ALCPlayerController::Server_RequestReturnToBase_Implementation()
{
    ACharacter* Char = GetCharacter();
    Char->SetActorHiddenInGame(false);

    // 베이스로 돌아가는 로직 구현
    // 예: 특정 레벨로 이동하거나 게이트 액터에 요청
    if (IsValid(LinkedGateActor))
    {
        // LinkedGateActor에 베이스로 돌아가는 함수가 있다면 호출
       // LinkedGateActor->ReturnToBaseCamp(this);
    }
}

void ALCPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALCPlayerController, LinkedSequenceActor);
}