#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "VoiceOptionWidget.generated.h"

class USlider;
class UButton;
class UTextBlock;
class UComboBoxString;

UENUM(BlueprintType)
enum class EVoiceInputMode : uint8
{
	Toggle UMETA(DisplayName = "토글"),
	Hold UMETA(DisplayName = "홀드"),
	Always UMETA(DisplayName = "상시오픈"),
	Off UMETA(DisplayName = "끄기")
};

UCLASS()
class LASTCANARY_API UVoiceOptionWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:

public:
	UPROPERTY(BlueprintReadwrite, EditAnywhere, Category = "VoiceOptionWidget")
	EVoiceInputMode CurrentInputMode;
	UPROPERTY(meta = (BindWidget))
	USlider* VoiceVolumeSlider;
	UPROPERTY(meta = (BindWidget))
	USlider* MyMicVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* VoiceVolumeText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MyMicVolumeText;
	UPROPERTY(BlueprintReadwrite, EditAnywhere, meta = (BindWidget))
	UTextBlock* InputModeText;

	UPROPERTY(meta = (BindWidget))
	UButton* VoiceInputModeButtonPre;
	UPROPERTY(meta = (BindWidget))
	UButton* VoiceInputModeButtonNext;
	
	UFUNCTION()
	void OnMicVolumeChanged(float Value);
	UFUNCTION()
	void OnVoiceVolumeChanged(float Value);

	UFUNCTION(BlueprintCallable)
	void RefreshInputModeText();

	void InitializeVoiceChatVolume();
	void InitializeMicrophoneVolume();
	void InitializeAllOptions();
};