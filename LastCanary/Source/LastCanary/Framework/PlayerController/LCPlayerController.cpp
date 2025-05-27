#include "Framework/PlayerController/LCPlayerController.h"

#include "Framework/GameMode/BaseGameMode.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

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
