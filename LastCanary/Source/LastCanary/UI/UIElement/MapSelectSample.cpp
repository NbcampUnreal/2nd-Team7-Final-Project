#include "UI/UIElement/MapSelectSample.h"
#include "Framework/GameState/LCGameState.h"
#include "DataTable/MapDataRow.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameMode/LCRoomGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UMapSelectSample::NativeConstruct()
{
	Super::NativeConstruct();

	if (!MapComboBox) return;
	MapComboBox->ClearOptions();

	if (ALCGameState* LCGS = GetLCGameState())
	{
		if (UDataTable* MapDatas = LCGS->GetMapData())
		{
			static const FString ContextString(TEXT("URoomSelectionWidget"));

			for (auto Map : MapDatas->GetRowNames())
			{
				if (const FMapDataRow* Row = MapDatas->FindRow<FMapDataRow>(Map, ContextString))
				{
					MapComboBox->AddOption(Row->MapInfo.MapName.ToString());
				}
			}

			if (MapComboBox->GetOptionCount() > 0)
			{
				MapComboBox->SetSelectedIndex(LCGS->GetSelectedMapIndex());
			}

			MapComboBox->OnSelectionChanged.AddDynamic(this, &UMapSelectSample::OnMapSelectionChanged);
		}
	}

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMapSelectSample::OnStartButtonClicked);
	}
}

void UMapSelectSample::OnMapSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectType)
{
	if (ALCGameState* LCGS = GetLCGameState())
	{
		int32 Index = MapComboBox->GetSelectedIndex();
		LCGS->SetMapIndex(Index);

		if (UDataTable* MapDatas = LCGS->GetMapData())
		{
			static const FString ContextString(TEXT("URoomSelectionWidget"));

			const FMapDataRow* Row = MapDatas->FindRow<FMapDataRow>(FName(SelectedItem), ContextString);

			if (Row && Row->MapInfo.MapThumbnail.IsValid())
			{
				UTexture2D* LoadedTexture = Row->MapInfo.MapThumbnail.LoadSynchronous();
				if (LoadedTexture && Thumnail)
				{
					Thumnail->SetBrushFromTexture(LoadedTexture, true);
				}
			}
		}
	}
}

void UMapSelectSample::OnStartButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(PC))
		{
			//RoomPC->StartGame();
		}
	}
}

ALCGameState* UMapSelectSample::GetLCGameState() const
{
	if (UWorld* W = GetWorld())
	{
		return Cast<ALCGameState>(W->GetGameState());
	}
	return nullptr;
}
