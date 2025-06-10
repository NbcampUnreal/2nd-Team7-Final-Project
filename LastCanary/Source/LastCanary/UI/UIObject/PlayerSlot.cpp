#include "UI/UIObject/PlayerSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "DataType/SessionPlayerInfo.h"
#include "GameFramework/PlayerState.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Kismet/GameplayStatics.h"

void UPlayerSlot::NativeConstruct()
{
    Super::NativeConstruct();

    if (KickButton)
    {
        KickButton->OnClicked.AddUniqueDynamic(this, &UPlayerSlot::OnKickButtonClicked);
    }
    //if (MuteButton)
    //{
    //    MuteButton->OnClicked.AddUniqueDynamic(this, &UPlayerSlot::OnMuteButtonClicked);
    //}
    //if (VolumeSlider)
    //{
    //    VolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UPlayerSlot::OnVolumeSliderValueChanged);
    //}
}

void UPlayerSlot::NativeDestruct()
{
    Super::NativeDestruct();

    if (KickButton)
    {
        KickButton->OnClicked.RemoveDynamic(this, &UPlayerSlot::OnKickButtonClicked);
    }
    //if (MuteButton)
    //{
    //    MuteButton->OnClicked.RemoveDynamic(this, &UPlayerSlot::OnMuteButtonClicked);
    //}
    //if (VolumeSlider)
    //{
    //    VolumeSlider->OnValueChanged.RemoveDynamic(this, &UPlayerSlot::OnVolumeSliderValueChanged);
    //}
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

    //if (ReadyText)
    //{
    //    FString IsReady = PlayerInfo.bIsPlayerReady ? "Ready" : "Not Ready";
    //    ReadyText->SetText(FText::FromString(IsReady));
    //}

    SetVisibleKickButton(PlayerInfo);

    SetVisibleVoiceChat(PlayerInfo);
}

void UPlayerSlot::SetVisibleKickButton(const FSessionPlayerInfo& PlayerInfo)
{
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

void UPlayerSlot::SetVisibleVoiceChat(const FSessionPlayerInfo& PlayerInfo)
{
    if (APlayerState* PS = GetOwningPlayer()->PlayerState)
    {
        if (PlayerInfo.PlayerName == PS->GetPlayerName())
        {
            MuteButton->SetVisibility(ESlateVisibility::Hidden);
            VolumeSlider->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            MuteButton->SetVisibility(ESlateVisibility::Visible);
            VolumeSlider->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void UPlayerSlot::OnKickButtonClicked()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ALCGameMode* GM = World->GetAuthGameMode<ALCGameMode>();
    if (!GM)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnKickButtonClicked: ALCGameMode을(를) 찾을 수 없습니다."));
        return;
    }

    GM->KickPlayer(MySessionPlayerInfo, FText::FromString("호스트에 의해 강퇴되었습니다!!"));
}

//
//void UPlayerSlot::OnMuteButtonClicked_Implementation()
//{
//}
//
//void UPlayerSlot::OnVolumeSliderValueChanged_Implementation(float Value)
//{
//}



void UPlayerSlot::ClearPlayerInfo()
{
    if (NickNameText)
    {
        NickNameText->SetText(FText::FromString("Empty Slot"));
    }
    if (KickButton)
    {
        KickButton->SetVisibility(ESlateVisibility::Hidden);
    }
    if (MuteButton)
    {
        MuteButton->SetVisibility(ESlateVisibility::Hidden);
    }
    if (VolumeSlider)
    {
        VolumeSlider->SetVisibility(ESlateVisibility::Hidden);
    }
}
