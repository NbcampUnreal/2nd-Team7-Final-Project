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
	if (HasAuthority() == false)
	{
		return;
	}

	Super::UseItem();

	InitializeNoteImageIndex();

	if (ItemData.NoteContent.IsEmpty())
	{
		LOG_Frame_WARNING(TEXT("[ANoteItem::UseItem] 쪽지 내용이 없습니다."));
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = IsValid(OwnerPawn) ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (IsValid(PC) == false)
	{
		LOG_Frame_WARNING(TEXT("[ANoteItem::UseItem] 유효한 컨트롤러를 찾지 못했습니다."));
		return;
	}

	// 클라이언트에게 UI 요청
	Client_ShowNotePopup(ItemData.NoteContent, ItemData.CandidateNoteImages, SelectedNoteImageIndex);
}

void ANoteItem::Client_ShowNotePopup_Implementation(const FText& Content, const TArray<TSoftObjectPtr<UTexture2D>>& Images, int32 ImageIndex)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = IsValid(OwnerPawn) ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

	if (IsValid(PC) == false || PC->IsLocalController() == false)
	{
		return;
	}

	if (ULCGameInstance* LCGameInstance = GetGameInstance<ULCGameInstance>())
	{
		if (ULCGameInstanceSubsystem* LCGameInstanceSubsystem = LCGameInstance->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			if (ULCUIManager* UIManager = LCGameInstanceSubsystem->GetUIManager())
			{
				UIManager->ShowNotePopup(Content, Images, ImageIndex);
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