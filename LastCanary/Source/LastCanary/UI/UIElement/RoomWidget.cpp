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

	if (InviteButton)
	{
		InviteButton->OnClicked.AddUniqueDynamic(this, &URoomWidget::OnInviteButtonClicked);
	}

	UpdatePlayerSlots(SessionPlayerInfos);
}

void URoomWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (InviteButton)
	{
		InviteButton->OnClicked.RemoveDynamic(this, &URoomWidget::OnInviteButtonClicked);
	}
}


void URoomWidget::CreatePlayerSlots()
{
	//if (!PlayerSlotClass)
	//{
	//	LOG_Frame_ERROR(TEXT("URoomWidget::CreatePlayerSlots: PlayerSlotClass가 바인드되어 있지 않습니다!"));
	//	return;
	//}

	//// 기존 슬롯 지우기
	//if (!IsValid(PlayerListContainer))
	//{
	//	LOG_Server_ERROR(TEXT("Player List Container Is Null!!"));
	//	return;
	//}

	PlayerListContainer->ClearChildren();
	PlayerSlots.Empty();

	// 슬롯 개수만큼 Player Slot 생성
	for (int32 i = 0; i < MaxPlayerNum; ++i)
	{
		UPlayerSlot* NewPlayerSlot = CreateWidget<UPlayerSlot>(this, PlayerSlotClass);

		if (NewPlayerSlot)
		{
			NewPlayerSlot->SetSlotIndex(i + 1);
			PlayerListContainer->AddChild(NewPlayerSlot);
			PlayerSlots.Add(NewPlayerSlot);
		}
	}
}

void URoomWidget::UpdatePlayerLists(const TArray<FSessionPlayerInfo>& PlayerInfos)
{
	SessionPlayerInfos = PlayerInfos;

	UpdatePlayerSlots(PlayerInfos);
}

void URoomWidget::UpdatePlayerSlots(const TArray<FSessionPlayerInfo>& PlayerInfos)
{
	if (!IsValid(PlayerListContainer))
	{
		LOG_Server_WARNING(TEXT("PlayerList Container is Not Valid!"));
		return;
	}

	for (int32 i = 0; i < PlayerSlots.Num(); ++i)
	{
		if (PlayerSlots[i])
		{
			PlayerSlots[i]->ClearPlayerInfo();

			if (PlayerInfos.IsValidIndex(i))
			{
				PlayerSlots[i]->SetPlayerInfo(PlayerInfos[i]);
			}
		}
	}
}

void URoomWidget::OnInviteButtonClicked()
{
	LOG_Frame_WARNING(TEXT("OnClick Invite Button"));
}