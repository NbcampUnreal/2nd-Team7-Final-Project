#include "UI/UIElement/RoomWidget.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void URoomWidget::NativeConstruct()
{
	Super::NativeConstruct();	
	//UpdatePlayerNames();
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
