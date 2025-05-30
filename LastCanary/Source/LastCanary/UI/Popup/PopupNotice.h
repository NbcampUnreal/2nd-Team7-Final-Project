#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "PopupNotice.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class LASTCANARY_API UPopupNotice : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Popup")
	void InitializeNoticePopup(const FString& Notice);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NoticeText;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UFUNCTION(BlueprintCallable)
	void OnExitButtonClicked();
};
