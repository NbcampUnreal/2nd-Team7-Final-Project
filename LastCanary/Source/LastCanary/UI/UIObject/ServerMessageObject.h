#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ServerMessageObject.generated.h"

class UTextBlock;

UCLASS()
class LASTCANARY_API UServerMessageObject : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* DisplayMessageAnim;

	UFUNCTION()
	void OnDisplayMessageAnimFinished();

public:
	void SetMessage(const FString& Message);
};
