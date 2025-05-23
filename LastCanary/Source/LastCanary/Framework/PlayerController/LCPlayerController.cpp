// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/PlayerController/LCPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Blueprint/UserWidget.h"


void ALCPlayerController::BeginPlay()
{
    Super::BeginPlay();

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
