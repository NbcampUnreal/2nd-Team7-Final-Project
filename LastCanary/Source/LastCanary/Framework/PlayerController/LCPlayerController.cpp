#include "Framework/PlayerController/LCPlayerController.h"

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
    bool bIsPlayerLoggedin = false;;
    if (const ULCGameInstance* MyGI = Cast<ULCGameInstance>(GetGameInstance()))
    {
        bIsPlayerLoggedin = MyGI->IsPlayerLoggedIn();
    }

    if (!bIsPlayerLoggedin)
    {
        ULCGameInstance* MyGI = Cast<ULCGameInstance>(GetGameInstance());
        check(MyGI);
        MyGI->Login();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player already logged in!"));
    }
}

void ALCPlayerController::Client_UpdatePlayers_Implementation()
{
          

}
