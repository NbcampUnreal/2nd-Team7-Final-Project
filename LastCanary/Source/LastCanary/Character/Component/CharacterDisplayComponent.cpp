#include "Character/Component/CharacterDisplayComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PostProcessComponent.h"

UCharacterDisplayComponent::UCharacterDisplayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterDisplayComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializePostProcess();
}

void UCharacterDisplayComponent::ApplyBrightness(float NormalizedValue)
{
	if (!PostProcessComponent) return;

	NormalizedValue = FMath::Clamp(NormalizedValue, 0.0f, 1.0f);

	float BrightnessValue = MinBrightness * FMath::Pow((MaxBrightness / MinBrightness), NormalizedValue);

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
		PostProcessComponent->Settings.bOverride_AutoExposureBias = true;
		PostProcessComponent->Settings.AutoExposureBias = 0.0f;
		PostProcessComponent->Settings.bOverride_AutoExposureMinBrightness = true;
		PostProcessComponent->Settings.bOverride_AutoExposureMaxBrightness = true;
	}
}