#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GraphicsSettingPanel.generated.h"

/**
 * 
 */

class UGraphicsOption;
class UButton;
class UTextBlock;


UCLASS()
class LASTCANARY_API UGraphicsSettingPanel : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_MaxFPS;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_TextureQuality;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_ShadowQuality;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_IndirectLighting;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_ReflectionQuality;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_OverallQualiatyPreset;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_ViewDistance;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_AntiAliasing;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_VFXQuality;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_PostProcessingQuality;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_FoliageQuality;
	UPROPERTY(meta = (BindWidget))
	UGraphicsOption* Option_ShadingQuality;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_VSyncLeft;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_VSyncRight;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_HDRLeft;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_HDRRight;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_VSyncState;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextBlock_HDRState;

	bool bVSyncEnabled = false;
	bool bHDREnabled = false;

	UFUNCTION()
	void OnVSyncToggle();
	UFUNCTION()
	void OnHDRToggle();

	UFUNCTION()
	void UpdateToggleStates();


	UFUNCTION()
	void OnGraphicsOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnMaxFPSOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnTextureQualityOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnShadowQualityOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnIndirectLightingOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnReflectionQualityOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnOverallQualiatyPresetOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnViewDistanceOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnAntiAliasingOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnVFXQualityOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnPostProcessingQualityOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnFoliageQualityOptionChanged(FName OptionKey, int32 SelectedIndex);
	UFUNCTION()
	void OnShadingQualityOptionChanged(FName OptionKey, int32 SelectedIndex);

	
	int32 CalculateMinumumSettingIndex();
	
	UFUNCTION()
	void UpdateGraphicsSettings();
};
