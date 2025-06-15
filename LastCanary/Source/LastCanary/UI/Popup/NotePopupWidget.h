#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "NotePopupWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UButton;
class UImage;
class UTexture2D;
UCLASS()
class LASTCANARY_API UNotePopupWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NoteContentTextBlock;
	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;
	UPROPERTY(meta = (BindWidget))
	UImage* BackgroundImage;

	UPROPERTY(EditDefaultsOnly, Category = "Note")
	TArray<UTexture2D*> NoteBackgroundList;

	UFUNCTION()
	void OnCloseButtonClicked();

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
public:
	void ShowNoteContent(const FText& InText, const TArray<TSoftObjectPtr<UTexture2D>>& CandidateImages, int32 Index);
};
