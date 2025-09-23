#include "Character/Component/CameraRecoilComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/Component/CharacterADSComponent.h"
#include "Inventory/ToolbarInventoryComponent.h"

#include "Components/TimelineComponent.h"

#include "LastCanary.h"

UCameraRecoilComponent::UCameraRecoilComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraRecoilComponent::BeginPlay()
{
	Super::BeginPlay();

	if (RecoilCurve) 
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleRecoilFloat"));
		RecoilTimeline.AddInterpFloat(RecoilCurve, ProgressFunction);
	}
	FOnTimelineEvent FinishedFunction;
	FinishedFunction.BindUFunction(this, FName("OnRecoilFinished"));
	RecoilTimeline.SetTimelineFinishedFunc(FinishedFunction);
	RecoilTimeline.SetLooping(false);
	RecoilTimeline.SetPlayRate(1.0f);
}

void UCameraRecoilComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RecoilTimeline.TickTimeline(DeltaTime);
}

void UCameraRecoilComponent::ApplyRecoil(float RecoilAmount)
{
	if (!IsValid(GetPlayerController()))
	{
		return;
	}
	GunRecoilMultiplier = RecoilAmount;
	StartRecoil(RecoilAmount);
}

void UCameraRecoilComponent::StartRecoil(float RecoilAmount)
{
	RecoilTimeline.PlayFromStart();
}

void UCameraRecoilComponent::OnRecoilFinished()
{
	// 반동 끝났을 때 처리
}

void UCameraRecoilComponent::HandleRecoilFloat(float Value)
{
	if (!IsValid(GetBaseCharacter()) || !IsValid(GetPlayerController()))
	{
		return;
	}

	if (GetBaseCharacter()->ADSComponent->bIsADS) //조준 상태인가?
	{
		GetBaseCharacter()->AddControllerPitchInput(Value * ADSRecoilMultiplier * GunRecoilMultiplier);
	}
	else
	{
		GetBaseCharacter()->AddControllerPitchInput(Value * DefaultRecoilMultiplier * GunRecoilMultiplier);
	}
}
