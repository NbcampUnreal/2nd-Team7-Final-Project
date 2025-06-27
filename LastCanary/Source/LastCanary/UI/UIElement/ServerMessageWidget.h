#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ServerMessageWidget.generated.h"

class UScrollBox;
class UServerMessageObject;

UCLASS()
class LASTCANARY_API UServerMessageWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void AddMessage(const FString& Message);

	UPROPERTY(meta = (BindWidget))
	UScrollBox* MessageScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
	TSubclassOf<UServerMessageObject> MessageObjectClass;
};
