#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "EnterPasswordWidget.generated.h"

/**
 * 
 */
class UEditableTextBox;
class UButton;
UCLASS()
class LASTCANARY_API UEnterPasswordWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
	void Init(const FString& InRoomID);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordInput;
	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmButton;

private:
	FString RoomID;
	void OnConfirmButtonClicked();
};
