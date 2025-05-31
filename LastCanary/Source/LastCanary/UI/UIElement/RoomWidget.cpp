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
	//UpdatePlayerNames();
	//CreatePlayerSlots();
}

void URoomWidget::CreatePlayerSlots()
{
	// 기존 슬롯 지우기
	PlayerListContainer->ClearChildren();
	PlayerNameTextBlocks.Empty();

	//if (UGameInstance* GI = GetGameInstance())
	//{
	//	if (ULCGameInstance* LCGI = Cast<ULCGameInstance>(GI))
	//	{
	//		for (int i = 0; i < LCGI->CurrentSessionInfo.MaxPlayerCount; i++)
	//		{
	//			UPlayerSlot* NewPlayerSlot = CreateWidget<UPlayerSlot>(this->GetOwningPlayer(), PlayerSlotClass);
	//			NewPlayerSlot->SetPlayerName("Empty Slot");

	//			PlayerListContainer->AddChild(NewPlayerSlot);
	//			PlayerNameTextBlocks.Add(NewPlayerSlot);
	//		}
	//	}
	//}

	// 슬롯 개수만큼 TextBlock 생성
	//for (int32 i = 0; i < NumSlots; ++i)
	//{
		// WidgetTree 를 통해 생성
		//UTextBlock* NewText = CreateWidget<UTextBlock>(this);
		//NewText->SetText(FText::FromString(TEXT("Empty Slot")));
		//// VerticalBox 에 추가
		//PlayerListContainer->AddChild(NewText);
		//// 배열에도 저장
		//PlayerNameTextBlocks.Add(NewText);
	//}
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

			switch (i)
			{
			case 0:
				if (PlayerNameText_0) PlayerNameText_0->SetText(FText::FromString(Name));
				break;
			case 1:
				if (PlayerNameText_1) PlayerNameText_1->SetText(FText::FromString(Name));
				break;
			case 2:
				if (PlayerNameText_2) PlayerNameText_2->SetText(FText::FromString(Name));
				break;
			case 3:
				if (PlayerNameText_3) PlayerNameText_3->SetText(FText::FromString(Name));
				break;
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
	const FString Empty = TEXT("Empty Slot");

	PlayerListContainer->ClearChildren();
	PlayerNameTextBlocks.Empty();

	int Index = 0;
	for (auto info : PlayerInfos)
	{
		Index++;

		UPlayerSlot* NewPlayerSlot = CreateWidget<UPlayerSlot>(this->GetOwningPlayer(), PlayerSlotClass);
		NewPlayerSlot->SetPlayerInfo(Index, info.PlayerName);

		PlayerListContainer->AddChild(NewPlayerSlot);
		PlayerNameTextBlocks.Add(NewPlayerSlot);
	}
	//for (int i = 0; i < PlayerInfos.Num(); i++)
	//{
	//	UPlayerSlot* NewPlayerSlot = CreateWidget<UPlayerSlot>(this->GetOwningPlayer(), PlayerSlotClass);
	//	NewPlayerSlot->SetPlayerInfo(i + 1, "Empty Slot");

	//	PlayerListContainer->AddChild(NewPlayerSlot);
	//	PlayerNameTextBlocks.Add(NewPlayerSlot);
	//}

	//for (int32 i = 0; i < PlayerNameTextBlocks.Num(); ++i)
	//{
	//	UTextBlock* TB = PlayerNameTextBlocks[i];
	//	if (!TB) continue;

	//	// PlayerInfos 에 값이 있으면 그 이름, 없으면 빈 슬롯
	//	const FString& Name = PlayerInfos.IsValidIndex(i) ? PlayerInfos[i].PlayerName : Empty;
	//	TB->SetText(FText::FromString(Name));
	//}
}
