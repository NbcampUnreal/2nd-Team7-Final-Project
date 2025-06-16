#include "Framework/PlayerController/LCPlayerController.h"

#include "Framework/GameMode/LCGameMode.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/Manager/LCCheatManager.h"

#include "Kismet/GameplayStatics.h"

#include "Blueprint/UserWidget.h"
#include "UI/Manager/LCUIManager.h"
#include "LastCanary.h"

ALCPlayerController::ALCPlayerController()
{
    CheatClass = ULCCheatManager::StaticClass();
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
