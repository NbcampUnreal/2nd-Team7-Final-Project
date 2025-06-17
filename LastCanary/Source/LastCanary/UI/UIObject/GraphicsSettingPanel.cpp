#include "UI/UIObject/GraphicsSettingPanel.h"
#include "UI/UIObject/GraphicsOption.h"
#include "GraphicsSettingPanel.h"

#include "LastCanary.h"

void UGraphicsSettingPanel::NativeConstruct()
{
	Super::NativeConstruct();

	const FName TableId = FName("/Game/_LastCanary/DataAsset/StringTable/ST_GraphicsOption");

	TArray<FText> QualityLevels = {
		FText::FromStringTable(TableId, FTextKey("Low")),
		FText::FromStringTable(TableId, FTextKey("Mid")),
		FText::FromStringTable(TableId, FTextKey("High")),
		FText::FromStringTable(TableId, FTextKey("Epic"))
	};

	if (Option_MaxFPS)
	{
		Option_MaxFPS->InitializeOption("MaxFPS", { 
			FText::FromString("30 FPS"), 
			FText::FromString("60 FPS"), 
			FText::FromString("120 FPS"), 
			FText::FromString("144 FPS")}, 0, FText::FromStringTable(TableId, FTextKey("MaxFPS")));
		Option_MaxFPS->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_TextureQuality)
	{	
		Option_TextureQuality->InitializeOption("TextureQuality", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("TextureQuality")));
		Option_TextureQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_ShadowQuality)
	{
		Option_ShadowQuality->InitializeOption("ShadowQuality", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("ShadowQuality")));
		Option_ShadowQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_IndirectLighting)
	{
		Option_IndirectLighting->InitializeOption("IndirectLighting", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("IndirectLighting")));
		Option_IndirectLighting->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_ReflectionQuality)
	{
		Option_ReflectionQuality->InitializeOption("ReflectionQuality", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("ReflectionQuality")));
		Option_ReflectionQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_OverallQualiatyPreset)
	{
		Option_OverallQualiatyPreset->InitializeOption("OverallQualityPreset", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("OverallQualityPreset")));
		Option_OverallQualiatyPreset->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_ViewDistance)
	{
		Option_ViewDistance->InitializeOption("ViewDistance", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("ViewDistance")));
		Option_ViewDistance->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_AntiAliasing)
	{
		Option_AntiAliasing->InitializeOption("AntiAliasing", {
			FText::FromString("Off"),
			FText::FromString("FXAA"),
			FText::FromString("TAA") }, 0, FText::FromStringTable(TableId, FTextKey("AntiAliasing")));
		Option_AntiAliasing->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_VFXQuality)
	{
		Option_VFXQuality->InitializeOption("VFXQuality", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("VFXQuality")));
		Option_VFXQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_PostProcessingQuality)
	{
		Option_PostProcessingQuality->InitializeOption("PostProcessingQuality", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("PostProcessingQuality")));
		Option_PostProcessingQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_FoliageQuality)
	{
		Option_FoliageQuality->InitializeOption("FoliageQuality", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("FoliageQuality")));
		Option_FoliageQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	}
	if (Option_ShadingQuality)
	{
		Option_ShadingQuality->InitializeOption("ShadingQuality", QualityLevels, 0,
			FText::FromStringTable(TableId, FTextKey("ShadingQuality")));
		Option_ShadingQuality->OnOptionChanged.AddUniqueDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
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
	Option_MaxFPS->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_TextureQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_ShadowQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_IndirectLighting->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_ReflectionQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_OverallQualiatyPreset->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_ViewDistance->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_AntiAliasing->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_VFXQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_PostProcessingQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_FoliageQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	Option_ShadingQuality->OnOptionChanged.RemoveDynamic(this, &UGraphicsSettingPanel::OnGraphicsOptionChanged);
	LOG_Frame_WARNING(TEXT("그래픽 설정 패널 제거됨"));
}

void UGraphicsSettingPanel::UpdateToggleStates()
{
	if (TextBlock_VSyncState)
	{
		TextBlock_VSyncState->SetText(bVSyncEnabled ? FText::FromString("On") : FText::FromString("Off"));
	}
	if (TextBlock_HDRState)
	{
		TextBlock_HDRState->SetText(bHDREnabled ? FText::FromString("On") : FText::FromString("Off"));
	}
}

void UGraphicsSettingPanel::OnVSyncToggle()
{
	bVSyncEnabled = !bVSyncEnabled;
	//TODO : VSync 설정 반영
	UpdateToggleStates();
}

void UGraphicsSettingPanel::OnHDRToggle()
{
	bHDREnabled = !bHDREnabled;
	//TODO : HDR 설정 반영
	UpdateToggleStates();
}

void UGraphicsSettingPanel::OnGraphicsOptionChanged(FName OptionKey, int32 SelectedIndex)
{
	//TODO : 여기에 그래픽 설정 반영하면 됩니다
	LOG_Frame_WARNING(TEXT("옵션 변경됨 : %s, %d"), *OptionKey.ToString(), SelectedIndex)
}
