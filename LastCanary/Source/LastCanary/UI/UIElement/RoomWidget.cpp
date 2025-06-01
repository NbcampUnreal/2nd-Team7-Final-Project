#include "UI/UIElement/RoomWidget.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UIObject/PlayerSlot.h"

#include "LastCanary.h"

void URoomWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreatePlayerSlots();

	if (InviteButton)
	{
		InviteButton->OnClicked.AddUniqueDynamic(this, &URoomWidget::OnInviteButtonClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddUniqueDynamic(this, &URoomWidget::OnBackButtonClicked);
	}
}

void URoomWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (InviteButton)
	{
		InviteButton->OnClicked.RemoveDynamic(this, &URoomWidget::OnInviteButtonClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(this, &URoomWidget::OnBackButtonClicked);
	}
}

void URoomWidget::CreatePlayerSlots()
{
	// 기존 슬롯 지우기
	PlayerListContainer->ClearChildren();
	PlayerSlots.Empty();

	// 슬롯 개수만큼 TextBlock 생성
	for (int32 i = 0; i < MaxPlayerNum; ++i)
	{
		UPlayerSlot* NewPlayerSlot = CreateWidget<UPlayerSlot>(this->GetOwningPlayer(), PlayerSlotClass);
		NewPlayerSlot->SetSlotIndex(i + 1);

		PlayerListContainer->AddChild(NewPlayerSlot);
		PlayerSlots.Add(NewPlayerSlot);
	}
}


void URoomWidget::UpdatePlayerNames()
{
	if (AGameStateBase* GS = UGameplayStatics::GetGameState(this))
	{
		const TArray<APlayerState*>& PlayerArray = GS->PlayerArray;

		// 임시로 최대 4명만 표시
		for (int32 i = 0; i < 4; ++i)
		{
			FString Name = TEXT("Empty Slot");

			if (i < PlayerArray.Num() && PlayerArray[i])
			{
				Name = PlayerArray[i]->GetPlayerName();
			}
		}
	}
}

void URoomWidget::UpdatePlayerLists(const TArray<FSessionPlayerInfo>& PlayerInfos)
{
	if (!IsValid(PlayerListContainer))
	{
		LOG_Server_WARNING(TEXT("PlayerList Container is Not Valid!"));
	}

	for (int32 i = 0; i < MaxPlayerNum; i++)
	{
		if (PlayerSlots.IsValidIndex(i) && PlayerSlots[i])
		{
			PlayerSlots[i]->ClearPlayerInfo();

			if (PlayerInfos.IsValidIndex(i))
			{
				FSessionPlayerInfo Info = PlayerInfos[i];
				PlayerSlots[i]->SetPlayerInfo(Info);
			}
		}
	}
}

void URoomWidget::OnInviteButtonClicked()
{
	LOG_Frame_WARNING(TEXT("OnClick Invite Button"));
}

void URoomWidget::OnBackButtonClicked()
{
	LOG_Frame_WARNING(TEXT("OnClick Back Button"));

}
