#include "Framework/PlayerController/LCPlayerController.h"

#include "Framework/GameMode/LCGameMode.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "Character/BasePlayerState.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"

#include "Framework/Manager/LCCheatManager.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIElement/RoomWidget.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"


ALCPlayerController::ALCPlayerController()
{
    CheatClass = ULCCheatManager::StaticClass();
}

void ALCPlayerController::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();

    LOG_Frame_WARNING(TEXT("PostSeamlessTravel: Ensuring CheatManager is ready"));

    if (CheatManager == nullptr)
    {
        CheatManager = NewObject<ULCCheatManager>(this, CheatClass);
        CheatManager->InitCheatManager();
    }

    LOG_Frame_WARNING(TEXT("PostSeamlessTravel: %s 호출 - IsLocalController: %d"), *GetName(), IsLocalController());
    GetWorldTimerManager().SetTimerForNextTick(this, &ALCPlayerController::DelayedPostTravelSetup);
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
        FInputModeGameAndUI GameAndUIInputMode;
        SetInputMode(GameAndUIInputMode);
    }
    else
    {
        LCUIManager->HideRoomWidget();
        FInputModeGameOnly GameInputMode;
        SetInputMode(GameInputMode);
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
		LCUIManager->ShowPopupNotice(FText::FromString(Message));
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
