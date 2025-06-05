#include "UI/UIObject/PlayerSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "DataType/SessionPlayerInfo.h"
#include "GameFramework/PlayerState.h"
#include "Framework/GameMode/BaseGameMode.h"
#include "Kismet/GameplayStatics.h"

void UPlayerSlot::NativeConstruct()
{
    Super::NativeConstruct();

    if (KickButton)
    {
        KickButton->OnClicked.AddUniqueDynamic(this, &UPlayerSlot::OnKickButtonClicked);
    }
}

void UPlayerSlot::NativeDestruct()
{
    Super::NativeDestruct();

    if (KickButton)
    {
        KickButton->OnClicked.RemoveDynamic(this, &UPlayerSlot::OnKickButtonClicked);
    }
}

void UPlayerSlot::SetSlotIndex(const int Index)
{
    if (IndexText)
    {
        IndexText->SetText(FText::AsNumber(Index));
    }
}

void UPlayerSlot::SetPlayerInfo(const FSessionPlayerInfo& PlayerInfo)
{
    MySessionPlayerInfo = PlayerInfo;

    if (NickNameText)
    {
        NickNameText->SetText(FText::FromString(PlayerInfo.PlayerName));
    }

    if (ReadyText)
    {
        FString IsReady = PlayerInfo.bIsPlayerReady ? "Ready" : "Not Ready";
        ReadyText->SetText(FText::FromString(IsReady));
    }

    if (!GetOwningPlayer()->HasAuthority())
    {
        KickButton->SetVisibility(ESlateVisibility::Hidden);
    }
    else
    {
        if (APlayerState* PS = GetOwningPlayer()->PlayerState)
        {
            if (PlayerInfo.PlayerName == PS->GetPlayerName())
            {
                KickButton->SetVisibility(ESlateVisibility::Hidden);
            }
            else
            {
                KickButton->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}

void UPlayerSlot::OnKickButtonClicked()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ABaseGameMode* GM = World->GetAuthGameMode<ABaseGameMode>();
    if (!GM)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnKickButtonClicked: ABaseGameMode을(를) 찾을 수 없습니다."));
        return;
    }

    GM->KickPlayer(MySessionPlayerInfo, FText::FromString("호스트에 의해 강퇴되었습니다!!"));
}

void UPlayerSlot::ClearPlayerInfo()
{
    NickNameText->SetText(FText::FromString("Empty Slot"));
    KickButton->SetVisibility(ESlateVisibility::Hidden);
}
