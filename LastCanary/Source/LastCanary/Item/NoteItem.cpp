#include "Item/NoteItem.h"
#include "UI/Manager/LCUIManager.h"
#include "Kismet/GameplayStatics.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "Character/BasePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

#include "LastCanary.h"

void ANoteItem::UseItem()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC->IsLocalController() == false)
	{
		return;
	}

	Super::UseItem();

	InitializeNoteImageIndex();

	// NoteContent가 비어있다면 무시
	if (ItemData.NoteContent.IsEmpty())
	{
		LOG_Frame_WARNING(TEXT("[ANoteItem::UseItem] 쪽지 내용이 없습니다."));
		return;
	}

	if (ULCGameInstance* LCGameInstance = GetGameInstance<ULCGameInstance>())
	{
		if (ULCGameInstanceSubsystem* LCGameInstanceSubsystem = LCGameInstance->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			if (ULCUIManager* UIManager = LCGameInstanceSubsystem->GetUIManager())
			{
				UIManager->ShowNotePopup(ItemData.NoteContent, ItemData.CandidateNoteImages, SelectedNoteImageIndex);
			}
		}
	}
}

void ANoteItem::InitializeNoteImageIndex()
{
	if (SelectedNoteImageIndex >= 0)
	{
		return;
	}

	const int32 CandidateCount = ItemData.CandidateNoteImages.Num();
	if (CandidateCount > 0)
	{
		SelectedNoteImageIndex = FMath::RandRange(0, CandidateCount - 1);
	}
}

void ANoteItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANoteItem, SelectedNoteImageIndex);
}