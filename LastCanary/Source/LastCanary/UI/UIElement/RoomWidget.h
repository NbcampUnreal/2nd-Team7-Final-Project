#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "RoomWidget.generated.h"

class UTextBlock;
UCLASS()
class LASTCANARY_API URoomWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	// TODO : 별도의 Widget을 최대 인원수 만큼 만들기
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText_0;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText_1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText_2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText_3;

	UFUNCTION(BlueprintCallable)
	void UpdatePlayerNames();
	
};
