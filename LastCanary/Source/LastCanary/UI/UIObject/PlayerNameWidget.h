#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "PlayerNameWidget.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class LASTCANARY_API UPlayerNameWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetPlayerName(const FString& InName);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
};