#include "LCLuxChargeTrigger.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCLuxChargeTrigger::ALCLuxChargeTrigger()
	: CurrentCharge(0.f)
	, bIsReceivingLux(false)
	, bChargeCompleted(false)
	, ChargeRate(0.5f)
	, DecayRate(0.25f)
	, TickInterval(0.05f)
	, MaxLightIntensity(5000.f)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	ChargeLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ChargeLight"));
	ChargeLight->SetupAttachment(RootComponent);
	ChargeLight->SetIntensity(0.f);
	ChargeLight->SetVisibility(true);
	ChargeLight->SetVisibility(true);
}

void ALCLuxChargeTrigger::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(ChargeTimerHandle, this, &ALCLuxChargeTrigger::UpdateCharge, TickInterval, true);
}

void ALCLuxChargeTrigger::ActivateGimmick_Implementation()
{
	Super::ActivateGimmick_Implementation();

	IGimmickEffectInterface::Execute_TriggerEffect(this);
	//LOG_Art(Log, TEXT("LuxChargeTrigger ▶ ActivateGimmick 호출됨"));
}

void ALCLuxChargeTrigger::TriggerEffect_Implementation()
{
	if (bIsReceivingLux) return;

	bIsReceivingLux = true;

	//LOG_Art(Log, TEXT("LuxChargeTrigger / TriggerEffect 수신 시작"));

	if (HasAuthority())
	{
		Multicast_TriggerEffect();
	}
}

void ALCLuxChargeTrigger::Multicast_TriggerEffect_Implementation()
{
	if (!HasAuthority())
	{
		bIsReceivingLux = true;
		//LOG_Art(Log, TEXT("[클라] LuxChargeTrigger / TriggerEffect 수신 시작"));
	}
}

void ALCLuxChargeTrigger::StopEffect_Implementation()
{
	bIsReceivingLux = false;
	//LOG_Art(Log, TEXT("LuxChargeTrigger / StopEffect 수신 종료"));
}

void ALCLuxChargeTrigger::UpdateCharge()
{
	const float Delta = TickInterval;
	const float Rate = bIsReceivingLux ? ChargeRate : -DecayRate;
	const float PreviousCharge = CurrentCharge;
	CurrentCharge = FMath::Clamp(CurrentCharge + Rate * Delta, 0.f, 1.f);

	/*LOG_Art(Log, TEXT("LuxChargeTrigger : 현재 상태: %s | 이전 차지: %.3f / 현재 차지: %.3f"),
		bIsReceivingLux ? TEXT("빛 수신 중") : TEXT("방전 중"),
		PreviousCharge, CurrentCharge);*/

	if (IsValid(ChargeLight))
	{
		const float NewIntensity = MaxLightIntensity * CurrentCharge;
		ChargeLight->SetIntensity(NewIntensity);
		//LOG_Art(Log, TEXT("LuxChargeTrigger : 라이트 밝기 적용: %.1f"), NewIntensity);
	}

	if (!bChargeCompleted && CurrentCharge >= 1.0f)
	{
		bChargeCompleted = true;
		//LOG_Art(Log, TEXT("LuxChargeTrigger : 충전 완료 타겟 기믹 발동"));

		for (AActor* Target : LinkedTargets)
		{
			if (IsValid(Target) && Target->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
			{
				//LOG_Art(Log, TEXT("타겟 기믹: %s ▶ ActivateGimmick 호출"), *Target->GetName());
				ILCGimmickInterface::Execute_ActivateGimmick(Target);
			}
		}
	}

	if (bChargeCompleted && CurrentCharge <= 0.f)
	{
		bChargeCompleted = false;
		//LOG_Art(Log, TEXT("LuxChargeTrigger : 완전 방전 -> 상태 초기화"));
	}
}

