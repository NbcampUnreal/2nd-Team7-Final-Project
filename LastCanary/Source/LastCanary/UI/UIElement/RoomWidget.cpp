#include "UI/UIElement/RoomWidget.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UIObject/PlayerSlot.h"

#include "LastCanary.h"

void URoomWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreatePlayerSlots();
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

	int Index = 0;
	for (auto Info : PlayerInfos)
	{
		if (PlayerSlots.IsValidIndex(Index) && PlayerSlots[Index])
		{
			PlayerSlots[Index]->SetPlayerInfo(Info);
		}

		Index++;
	}
}
