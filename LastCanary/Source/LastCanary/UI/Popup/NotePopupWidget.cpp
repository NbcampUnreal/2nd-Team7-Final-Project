#include "UI/Popup/NotePopupWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "UI/Manager/LCUIManager.h"

void UNotePopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UNotePopupWidget::OnCloseButtonClicked);
	}
}

void UNotePopupWidget::NativeDestruct()
{
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UNotePopupWidget::OnCloseButtonClicked);
	}
	Super::NativeDestruct();
}

void UNotePopupWidget::OnCloseButtonClicked()
{
	ULCUIManager* UIManager = ResolveUIManager();
	if (UIManager)
	{
		UIManager->HideNotePopup();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManager is not available when closing NotePopupWidget."));
	}
}

FReply UNotePopupWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ULCUIManager* UIManager = ResolveUIManager();
		if (UIManager)
		{
			UIManager->HideNotePopup();
			UE_LOG(LogTemp, Warning, TEXT("NativeOnKeyDown. InKeyEvent.GetKey() == EKeys::Escape"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UIManager is not available when closing NotePopupWidget."));
			RemoveFromParent(); // fallback
		}

		// 키 입력을 처리했다는 의미로 Handled 반환
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UNotePopupWidget::ShowNoteContent(const FText& InText, const TArray<TSoftObjectPtr<UTexture2D>>& CandidateImages, int32 Index)
{
	if (NoteContentTextBlock)
	{
		NoteContentTextBlock->SetText(InText);
	}

	if (BackgroundImage && CandidateImages.IsValidIndex(Index))
	{
		UTexture2D* Texture = CandidateImages[Index].LoadSynchronous();
		if (Texture)
		{
			BackgroundImage->SetBrushFromTexture(Texture, true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("NotePopup: Failed to load texture at index %d"), Index);
		}
	}
}