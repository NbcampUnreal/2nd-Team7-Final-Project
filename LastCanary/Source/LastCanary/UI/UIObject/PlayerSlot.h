#pragma once

#include "CoreMinimal.h"
#include "DataType/SessionPlayerInfo.h"
#include "Blueprint/UserWidget.h"
#include "PlayerSlot.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class LASTCANARY_API UPlayerSlot : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* IndexText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NickNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* KickButton;

	void SetSlotIndex(const int Index);
	void ClearPlayerInfo();
	void SetPlayerInfo(const FSessionPlayerInfo& PlayerInfo);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FSessionPlayerInfo MySessionPlayerInfo;
};
