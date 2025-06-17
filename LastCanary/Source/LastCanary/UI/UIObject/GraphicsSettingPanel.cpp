#include "UI/UIObject/GraphicsSettingPanel.h"
#include "UI/UIObject/GraphicsOption.h"
#include "GraphicsSettingPanel.h"
#include "GameFramework/GameUserSettings.h"

#include "LastCanary.h"

void UGraphicsSettingPanel::NativeConstruct()
{
	Super::NativeConstruct();
	LOG_Item_WARNING(TEXT("그래픽 설정 이니셜라이즈."));

	const FName TableId = FName("/Game/_LastCanary/DataAsset/StringTable/ST_GraphicsOption");

	TArray<FText> QualityLevels = {
		FText::FromStringTable(TableId, FTextKey("Low")),
		FText::FromStringTable(TableId, FTextKey("Mid")),
		FText::FromStringTable(TableId, FTextKey("High")),
		FText::FromStringTable(TableId, FTextKey("Epic"))
	};

	//첫 로드시 게임 세팅에서 값을 가져와 UI에 연결
	int32 FrameIndex = 0;
	UGameUserSettings* Settings = GEngine->GetGameUserSettings();
	UE_LOG(LogTemp, Warning, TEXT("그래픽 세팅 적용"));
	float FramePerSecond = Settings->GetFrameRateLimit();
	if (FMath::IsNearlyEqual(30.0f, FramePerSecond, KINDA_SMALL_NUMBER))
	{
		FrameIndex = 0;
	}
	else if (FMath::IsNearlyEqual(60.0f, FramePerSecond, KINDA_SMALL_NUMBER))
	{
		FrameIndex = 1;
	}
	else if (FMath::IsNearlyEqual(120.0f, FramePerSecond, KINDA_SMALL_NUMBER))
	{
		FrameIndex = 2;
	}
	else if (FMath::IsNearlyEqual(144.0f, FramePerSecond, KINDA_SMALL_NUMBER))
	{
		FrameIndex = 3;
	}

	if (Option_MaxFPS)
	{
		Option_MaxFPS->InitializeOption("MaxFPS", { 
			FText::FromString("30 FPS"), 
			FText::FromString("60 FPS"), 
			FText::FromString("120 FPS"), 
			FText::FromString("144 FPS")}, FrameIndex, FText::FromStringTable(TableId, FTextKey("MaxFPS")));
		Option_MaxFPS->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnMaxFPSOptionChanged);
	}
	if (Option_TextureQuality)
	{	
		Option_TextureQuality->InitializeOption("TextureQuality", QualityLevels, Settings->GetTextureQuality(),
			FText::FromStringTable(TableId, FTextKey("TextureQuality")));
		Option_TextureQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnTextureQualityOptionChanged);
	}
	if (Option_ShadowQuality)
	{
		Option_ShadowQuality->InitializeOption("ShadowQuality", QualityLevels, Settings->GetShadowQuality(),
			FText::FromStringTable(TableId, FTextKey("ShadowQuality")));
		Option_ShadowQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnShadowQualityOptionChanged);
	}
	if (Option_IndirectLighting)
	{
		Option_IndirectLighting->InitializeOption("IndirectLighting", QualityLevels, Settings->GetGlobalIlluminationQuality(),
			FText::FromStringTable(TableId, FTextKey("IndirectLighting")));
		Option_IndirectLighting->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnIndirectLightingOptionChanged);
	}
	if (Option_ReflectionQuality)
	{
		Option_ReflectionQuality->InitializeOption("ReflectionQuality", QualityLevels, Settings->GetReflectionQuality(),
			FText::FromStringTable(TableId, FTextKey("ReflectionQuality")));
		Option_ReflectionQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnReflectionQualityOptionChanged);
	}
	if (Option_OverallQualiatyPreset)
	{
		int32 index = CalculateMinumumSettingIndex();
		Option_OverallQualiatyPreset->InitializeOption("OverallQualityPreset", QualityLevels, index,
			FText::FromStringTable(TableId, FTextKey("OverallQualityPreset")));
		Option_OverallQualiatyPreset->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnOverallQualiatyPresetOptionChanged);
	}
	if (Option_ViewDistance)
	{
		Option_ViewDistance->InitializeOption("ViewDistance", QualityLevels, Settings->GetViewDistanceQuality(),
			FText::FromStringTable(TableId, FTextKey("ViewDistance")));
		Option_ViewDistance->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnViewDistanceOptionChanged);
	}
	if (Option_AntiAliasing)
	{
		Option_AntiAliasing->InitializeOption("AntiAliasing", {
			FText::FromString("Off"),
			FText::FromString("FXAA"),
			FText::FromString("TAA"),
			FText::FromString("MSAA") }, Settings->GetAntiAliasingQuality(), FText::FromStringTable(TableId, FTextKey("AntiAliasing")));
		Option_AntiAliasing->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnAntiAliasingOptionChanged);
	}
	if (Option_VFXQuality)
	{
		Option_VFXQuality->InitializeOption("VFXQuality", QualityLevels, Settings->GetVisualEffectQuality(),
			FText::FromStringTable(TableId, FTextKey("VFXQuality")));
		Option_VFXQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnVFXQualityOptionChanged);
	}
	if (Option_PostProcessingQuality)
	{
		Option_PostProcessingQuality->InitializeOption("PostProcessingQuality", QualityLevels, Settings->GetPostProcessingQuality(),
			FText::FromStringTable(TableId, FTextKey("PostProcessingQuality")));
		Option_PostProcessingQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnPostProcessingQualityOptionChanged);
	}
	if (Option_FoliageQuality)
	{
		Option_FoliageQuality->InitializeOption("FoliageQuality", QualityLevels, Settings->GetFoliageQuality(),
			FText::FromStringTable(TableId, FTextKey("FoliageQuality")));
		Option_FoliageQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnFoliageQualityOptionChanged);
	}
	if (Option_ShadingQuality)
	{
		Option_ShadingQuality->InitializeOption("ShadingQuality", QualityLevels, Settings->GetShadingQuality(),
			FText::FromStringTable(TableId, FTextKey("ShadingQuality")));
		Option_ShadingQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnShadingQualityOptionChanged);
	}

	if (Button_VSyncRight)
	{
		Button_VSyncRight->OnClicked.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnVSyncToggle);
	}
	if (Button_VSyncLeft)
	{
		Button_VSyncLeft->OnClicked.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnVSyncToggle);
	}
	if (Button_HDRRight)
	{
		Button_HDRRight->OnClicked.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnHDRToggle);
	}
	if (Button_HDRLeft)
	{
		Button_HDRLeft->OnClicked.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnHDRToggle);
	}
	UpdateToggleStates();
	LOG_Frame_WARNING(TEXT("그래픽 설정 패널 초기화"));
}

void UGraphicsSettingPanel::NativeDestruct()
{
	Super::NativeDestruct();
	Option_MaxFPS->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnMaxFPSOptionChanged);
	Option_TextureQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnTextureQualityOptionChanged);
	Option_ShadowQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnShadowQualityOptionChanged);
	Option_IndirectLighting->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnIndirectLightingOptionChanged);
	Option_ReflectionQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnReflectionQualityOptionChanged);
	Option_OverallQualiatyPreset->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnOverallQualiatyPresetOptionChanged);
	Option_ViewDistance->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnViewDistanceOptionChanged);
	Option_AntiAliasing->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnAntiAliasingOptionChanged);
	Option_VFXQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnVFXQualityOptionChanged);
	Option_PostProcessingQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnPostProcessingQualityOptionChanged);
	Option_FoliageQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnFoliageQualityOptionChanged);
	Option_ShadingQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnShadingQualityOptionChanged);
	LOG_Frame_WARNING(TEXT("그래픽 설정 패널 제거됨"));
}

void UGraphicsSettingPanel::UpdateToggleStates()
{
	UGameUserSettings* Settings = GEngine->GetGameUserSettings();

	if (TextBlock_VSyncState)
	{
		bVSyncEnabled = Settings->IsVSyncEnabled();
		TextBlock_VSyncState->SetText(bVSyncEnabled ? FText::FromString("On") : FText::FromString("Off"));
	}
	if (TextBlock_HDRState)
	{
		bHDREnabled = Settings->IsHDREnabled();
		TextBlock_HDRState->SetText(bHDREnabled ? FText::FromString("On") : FText::FromString("Off"));
	}
}

void UGraphicsSettingPanel::OnVSyncToggle()
{
	bVSyncEnabled = !bVSyncEnabled;
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetVSyncEnabled(bVSyncEnabled);
		Settings->ApplySettings(false);
		Settings->SaveSettings();
	}
	
	
	//TODO : VSync 설정 반영
	UpdateToggleStates();
}

void UGraphicsSettingPanel::OnHDRToggle()
{
	bHDREnabled = !bHDREnabled;
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->EnableHDRDisplayOutput(bHDREnabled);
		Settings->ApplySettings(false);
		Settings->SaveSettings();
	}
	
	//TODO : HDR 설정 반영
	UpdateToggleStates();
}

void UGraphicsSettingPanel::OnGraphicsOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	//TODO : 여기에 그래픽 설정 반영하면 됩니다
	LOG_Frame_WARNING(TEXT("옵션 변경됨 : %s, %d"), *OptionKey.ToString(), SelectedIndex);
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		UE_LOG(LogTemp, Warning, TEXT("그래픽 세팅 적용"));
		float FramePerSecond = 0.0f;
		switch (Option_MaxFPS->GetCurrentIndex())
		{
			case 0: FramePerSecond = 30.0f; break;
			case 1: FramePerSecond = 60.0f; break;
			case 2: FramePerSecond = 120.0f; break;
			case 3: FramePerSecond = 144.0f; break;
			default: FramePerSecond = 0.0f; break;
		}
		Settings->SetFrameRateLimit(FramePerSecond);


		Settings->SetTextureQuality(Option_TextureQuality->GetCurrentIndex());
		Settings->SetShadowQuality(Option_ShadowQuality->GetCurrentIndex());
		Settings->SetGlobalIlluminationQuality(Option_IndirectLighting->GetCurrentIndex());
		Settings->SetReflectionQuality(Option_ReflectionQuality->GetCurrentIndex());
		Settings->SetOverallScalabilityLevel(Option_OverallQualiatyPreset->GetCurrentIndex());
		Settings->SetViewDistanceQuality(Option_ViewDistance->GetCurrentIndex());
		Settings->SetAntiAliasingQuality(Option_AntiAliasing->GetCurrentIndex());
		Settings->SetVisualEffectQuality(Option_VFXQuality->GetCurrentIndex());
		Settings->SetPostProcessingQuality(Option_PostProcessingQuality->GetCurrentIndex());
		Settings->SetFoliageQuality(Option_FoliageQuality->GetCurrentIndex());
		Settings->SetShadingQuality(Option_ShadingQuality->GetCurrentIndex());


		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnMaxFPSOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		UE_LOG(LogTemp, Warning, TEXT("그래픽 세팅 적용"));
		float FramePerSecond = 0.0f;
		switch (Option_MaxFPS->GetCurrentIndex())
		{
		case 0: FramePerSecond = 30.0f; break;
		case 1: FramePerSecond = 60.0f; break;
		case 2: FramePerSecond = 120.0f; break;
		case 3: FramePerSecond = 144.0f; break;
		default: FramePerSecond = 0.0f; break;
		}
		Settings->SetFrameRateLimit(FramePerSecond);
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnTextureQualityOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetTextureQuality(Option_TextureQuality->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnShadowQualityOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetShadowQuality(Option_ShadowQuality->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnIndirectLightingOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetGlobalIlluminationQuality(Option_IndirectLighting->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnReflectionQualityOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetReflectionQuality(Option_ReflectionQuality->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnOverallQualiatyPresetOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetOverallScalabilityLevel(Option_OverallQualiatyPreset->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnViewDistanceOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetViewDistanceQuality(Option_ViewDistance->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnAntiAliasingOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetAntiAliasingQuality(Option_AntiAliasing->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnVFXQualityOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetVisualEffectQuality(Option_VFXQuality->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnPostProcessingQualityOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetPostProcessingQuality(Option_PostProcessingQuality->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnFoliageQualityOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetFoliageQuality(Option_FoliageQuality->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::OnShadingQualityOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SetShadingQuality(Option_ShadingQuality->GetCurrentIndex());
		Settings->ApplySettings(false);
		UpdateGraphicsSettings();
		Settings->SaveSettings();
	}
}

void UGraphicsSettingPanel::UpdateGraphicsSettings()
{
	LOG_Item_WARNING(TEXT("그래픽 설정 이니셜라이즈."));

	const FName TableId = FName("/Game/_LastCanary/DataAsset/StringTable/ST_GraphicsOption");

	TArray<FText> QualityLevels = {
		FText::FromStringTable(TableId, FTextKey("Low")),
		FText::FromStringTable(TableId, FTextKey("Mid")),
		FText::FromStringTable(TableId, FTextKey("High")),
		FText::FromStringTable(TableId, FTextKey("Epic"))
	};

	//첫 로드시 게임 세팅에서 값을 가져와 UI에 연결
	int32 FrameIndex = 0;
	UGameUserSettings* Settings = GEngine->GetGameUserSettings();
	float FramePerSecond = Settings->GetFrameRateLimit();
	if (FMath::IsNearlyEqual(30.0f, FramePerSecond, KINDA_SMALL_NUMBER))
	{
		FrameIndex = 0;
	}
	else if (FMath::IsNearlyEqual(60.0f, FramePerSecond, KINDA_SMALL_NUMBER))
	{
		FrameIndex = 1;
	}
	else if (FMath::IsNearlyEqual(120.0f, FramePerSecond, KINDA_SMALL_NUMBER))
	{
		FrameIndex = 2;
	}
	else if (FMath::IsNearlyEqual(144.0f, FramePerSecond, KINDA_SMALL_NUMBER))
	{
		FrameIndex = 3;
	}

	if (Option_MaxFPS)
	{
		Option_MaxFPS->InitializeOption("MaxFPS", {
			FText::FromString("30 FPS"),
			FText::FromString("60 FPS"),
			FText::FromString("120 FPS"),
			FText::FromString("144 FPS") }, FrameIndex, FText::FromStringTable(TableId, FTextKey("MaxFPS")));
	}
	if (Option_TextureQuality)
	{
		Option_TextureQuality->InitializeOption("TextureQuality", QualityLevels, Settings->GetTextureQuality(),
			FText::FromStringTable(TableId, FTextKey("TextureQuality")));
	}
	if (Option_ShadowQuality)
	{
		Option_ShadowQuality->InitializeOption("ShadowQuality", QualityLevels, Settings->GetShadowQuality(),
			FText::FromStringTable(TableId, FTextKey("ShadowQuality")));
	}
	if (Option_IndirectLighting)
	{
		Option_IndirectLighting->InitializeOption("IndirectLighting", QualityLevels, Settings->GetGlobalIlluminationQuality(),
			FText::FromStringTable(TableId, FTextKey("IndirectLighting")));
	}
	if (Option_ReflectionQuality)
	{
		Option_ReflectionQuality->InitializeOption("ReflectionQuality", QualityLevels, Settings->GetReflectionQuality(),
			FText::FromStringTable(TableId, FTextKey("ReflectionQuality")));
	}
	if (Option_ViewDistance)
	{
		Option_ViewDistance->InitializeOption("ViewDistance", QualityLevels, Settings->GetViewDistanceQuality(),
			FText::FromStringTable(TableId, FTextKey("ViewDistance")));
	}
	if (Option_AntiAliasing)
	{
		Option_AntiAliasing->InitializeOption("AntiAliasing", {
			FText::FromString("Off"),
			FText::FromString("FXAA"),
			FText::FromString("TAA"),
			FText::FromString("MSAA") }, Settings->GetAntiAliasingQuality(), FText::FromStringTable(TableId, FTextKey("AntiAliasing")));
	}
	if (Option_VFXQuality)
	{
		Option_VFXQuality->InitializeOption("VFXQuality", QualityLevels, Settings->GetVisualEffectQuality(),
			FText::FromStringTable(TableId, FTextKey("VFXQuality")));
	}
	if (Option_PostProcessingQuality)
	{
		Option_PostProcessingQuality->InitializeOption("PostProcessingQuality", QualityLevels, Settings->GetPostProcessingQuality(),
			FText::FromStringTable(TableId, FTextKey("PostProcessingQuality")));
	}
	if (Option_FoliageQuality)
	{
		Option_FoliageQuality->InitializeOption("FoliageQuality", QualityLevels, Settings->GetFoliageQuality(),
			FText::FromStringTable(TableId, FTextKey("FoliageQuality")));
	}
	if (Option_ShadingQuality)
	{
		Option_ShadingQuality->InitializeOption("ShadingQuality", QualityLevels, Settings->GetShadingQuality(),
			FText::FromStringTable(TableId, FTextKey("ShadingQuality")));
	}
	if (Option_OverallQualiatyPreset)
	{
		int32 MinimumIndex = CalculateMinumumSettingIndex();
		Option_OverallQualiatyPreset->InitializeOption("OverallQualityPreset", QualityLevels, MinimumIndex, FText::FromStringTable(TableId, FTextKey("OverallQualityPreset")));
	}

	UpdateToggleStates();
	LOG_Frame_WARNING(TEXT("그래픽 설정 패널 업데이트"));
}

int32 UGraphicsSettingPanel::CalculateMinumumSettingIndex()
{
	int32 index = 4; // 제일 큰 거
	UGameUserSettings* Settings = GEngine->GetGameUserSettings();
	index = FMath::Min(index, Settings->GetTextureQuality());
	index = FMath::Min(index, Settings->GetShadowQuality());
	index = FMath::Min(index, Settings->GetGlobalIlluminationQuality());
	index = FMath::Min(index, Settings->GetReflectionQuality());
	index = FMath::Min(index, Settings->GetViewDistanceQuality());
	index = FMath::Min(index, Settings->GetAntiAliasingQuality());
	index = FMath::Min(index, Settings->GetVisualEffectQuality());
	index = FMath::Min(index, Settings->GetPostProcessingQuality());
	index = FMath::Min(index, Settings->GetShadingQuality());
	return index;
}
