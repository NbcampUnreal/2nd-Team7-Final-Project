#include "Framework/PlayerController/LCPlayerController.h"

#include "Framework/GameMode/BaseGameMode.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"

#include "Blueprint/UserWidget.h"
#include "UI/Manager/LCUIManager.h"


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

    Login();
}

void ALCPlayerController::Login()
{

}

void ALCPlayerController::Server_SetPlayerInfo_Implementation(const FSessionPlayerInfo& PlayerInfo)
{
    if (UWorld* World = GetWorld())
    {
        if (ABaseGameMode* LCGM = Cast<ABaseGameMode>(World->GetAuthGameMode()))
        {
            LCGM->SetPlayerInfo(PlayerInfo);
        }
    }
}

void ALCPlayerController::Client_UpdatePlayerList_Implementation(const TArray<FSessionPlayerInfo>& PlayerInfos)
{

}

void ALCPlayerController::ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason)
{
    Super::ClientReturnToMainMenuWithTextReason_Implementation(ReturnReason);

    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            // "GameSession" 이라는 이름으로 생성된 세션을 파괴
            Sessions->DestroySession(NAME_GameSession);
        }
    }
}

void ALCPlayerController::ClientWasKicked_Implementation_Implementation(const FText& KickReason)
{
    // BluePrint 에서 DestroySession => 새로운 세션에 Join하기 위해
}


void ALCPlayerController::Client_UpdatePlayers_Implementation()
{

}

void ALCPlayerController::StartGame(FString SoftPath)
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("Try Start Game!!"));
    if (UWorld* World = GetWorld())
    {
        if (ABaseGameMode* LCGM = Cast<ABaseGameMode>(World->GetAuthGameMode()))
        {
            LCGM->TravelMapBySoftPath(SoftPath);
        }
    }
}
