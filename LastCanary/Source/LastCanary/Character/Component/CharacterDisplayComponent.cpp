#include "Character/Component/CharacterDisplayComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PostProcessComponent.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"

#include "LastCanary.h"

UCharacterDisplayComponent::UCharacterDisplayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterDisplayComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializePostProcess();
	LoadBrightnessSetting();
}

void UCharacterDisplayComponent::ApplyBrightness(float NormalizedValue)
{
	if (!PostProcessComponent) return;
	LOG_Char_WARNING(TEXT("밝기 적용"));

	float BrightnessValue = MinBrightness * FMath::Pow((MaxBrightness / MinBrightness), NormalizedValue);
	LOG_Char_WARNING(TEXT("BrightnessValue  = %f"), BrightnessValue);

	PostProcessComponent->Settings.AutoExposureBias = BrightnessValue;
	PostProcessComponent->Settings.AutoExposureMinBrightness = BrightnessValue - 0.01f;
	PostProcessComponent->Settings.AutoExposureMaxBrightness = BrightnessValue + 0.01f;
}

void UCharacterDisplayComponent::InitializePostProcess()
{
	PostProcessComponent = NewObject<UPostProcessComponent>(GetOwner());
	if (PostProcessComponent)
	{
		PostProcessComponent->RegisterComponent();
		PostProcessComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		// 기본 세팅
		PostProcessComponent->bUnbound = true;
		
		PostProcessComponent->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
		PostProcessComponent->Settings.bOverride_AutoExposureBias = true;
		PostProcessComponent->Settings.AutoExposureBias = 0.0f;
		PostProcessComponent->Settings.bOverride_AutoExposureMinBrightness = true;
		PostProcessComponent->Settings.bOverride_AutoExposureMaxBrightness = true;

		// 블렌드 웨이트 1.0으로 보정 적용 보장
		PostProcessComponent->BlendWeight = 1.0f;
		PostProcessComponent->Priority = 100.0f;
	}
}

void UCharacterDisplayComponent::LoadBrightnessSetting()
{
	float Brightness = ULCLocalPlayerSaveGame::LoadBrightness(GetWorld());
	float Normalized = FMath::Clamp(Brightness, 0.0f, 1.0f);
	ApplyBrightness(Normalized);
}
