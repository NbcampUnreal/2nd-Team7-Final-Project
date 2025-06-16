#pragma once

#include "CoreMinimal.h"
#include "DataType/SessionPlayerInfo.h"
#include "UI/LCUserWidgetBase.h"
#include "PlayerSlot.generated.h"

class UTextBlock;
class UButton;
class USlider;

UCLASS()
class LASTCANARY_API UPlayerSlot : public ULCUserWidgetBase
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

	//UPROPERTY(meta = (BindWidget))
	//UTextBlock* ReadyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* KickButton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UButton* MuteButton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	USlider* VolumeSlider;

	void SetSlotIndex(const int Index);
	void ClearPlayerInfo();
	void SetPlayerInfo(const FSessionPlayerInfo& PlayerInfo);
	void SetVisibleKickButton(const FSessionPlayerInfo& PlayerInfo);
	void SetVisibleVoiceChat(const FSessionPlayerInfo& PlayerInfo);

	UFUNCTION()
	void OnKickButtonClicked();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FSessionPlayerInfo MySessionPlayerInfo;
};
