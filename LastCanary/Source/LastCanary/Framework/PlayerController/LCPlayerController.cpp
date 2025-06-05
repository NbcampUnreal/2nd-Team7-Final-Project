#include "Framework/PlayerController/LCPlayerController.h"

#include "Framework/GameMode/LCGameMode.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/Manager/LCCheatManager.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
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
    LCUIManager->ShowLoadingLevel();
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
