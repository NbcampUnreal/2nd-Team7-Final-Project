#include "LCLuxChargeTrigger.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCLuxChargeTrigger::ALCLuxChargeTrigger()
	: CurrentCharge(0.f)
	, bIsReceivingLux(false)
	, bChargeCompleted(false)
	, ChargeRate(0.1f)
	, DecayRate(0.1f)
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
}

void ALCLuxChargeTrigger::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(ChargeTimerHandle, this, &ALCLuxChargeTrigger::UpdateCharge, TickInterval, true);
}

void ALCLuxChargeTrigger::ActivateGimmick_Implementation()
{
	Super::ActivateGimmick_Implementation();

	//LOG_Art(Log, TEXT("ALCLuxChargeTrigger::ActivateGimmick_Implementation ▶ ActivateGimmick 호출됨 (직접 발동형 용도)"));
}

void ALCLuxChargeTrigger::TriggerEffect_Implementation()
{
	LastReceivedTime = GetWorld()->GetTimeSeconds(); 

	if (!bIsReceivingLux)
	{
		bIsReceivingLux = true;
		//LOG_Art(Log, TEXT("빛 수신 시작"));
	}

	if (HasAuthority())
	{
		Multicast_TriggerEffect();
	}
}

void ALCLuxChargeTrigger::Multicast_TriggerEffect_Implementation()
{
	if (!HasAuthority())
	{
		LastReceivedTime = GetWorld()->GetTimeSeconds(); 
		if (!bIsReceivingLux)
		{
			bIsReceivingLux = true;
			LOG_Art(Log, TEXT("[클라] 빛 수신 시작"));
		}
	}
}

void ALCLuxChargeTrigger::StopEffect_Implementation()
{
	/*LOG_Art(Log, TEXT("ALCLuxChargeTrigger::StopEffect_Implementation() 호출됨 | IsServer: %s"),
		HasAuthority() ? TEXT("서버") : TEXT("클라"));*/

	if (!bIsReceivingLux) return;

	bIsReceivingLux = false;
	//LOG_Art(Log, TEXT("ALCLuxChargeTrigger::StopEffect_Implementation 빛 수신 종료"));

	if (HasAuthority())
	{
		Multicast_StopEffect();
	}
}

void ALCLuxChargeTrigger::Multicast_StopEffect_Implementation()
{
	if (!HasAuthority())
	{
		bIsReceivingLux = false;
		//LOG_Art(Log, TEXT("ALCLuxChargeTrigger::Multicast_StopEffect_Implementation : [클라] 빛 수신 종료"));
	}
}

void ALCLuxChargeTrigger::UpdateCharge()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (bIsReceivingLux && CurrentTime - LastReceivedTime > GracePeriod)
	{
		bIsReceivingLux = false;
		//LOG_Art(Log, TEXT("LuxChargeTrigger : 빛 수신 중단 (유예 시간 초과)"));

		if (HasAuthority())
		{
			Multicast_StopEffect();
		}
	}

	const float Delta = TickInterval;
	const float Rate = bIsReceivingLux ? ChargeRate : -DecayRate;
	const float PreviousCharge = CurrentCharge;

	CurrentCharge = FMath::Clamp(CurrentCharge + Rate * Delta, 0.f, 1.f);

	/*LOG_Art(Log, TEXT("UpdateCharge ▶ 상태: %s | bIsReceivingLux: %s | Δ: %.3f | 이전: %.3f → 현재: %.3f"),
		bIsReceivingLux ? TEXT("충전 중") : TEXT("방전 중"),
		bIsReceivingLux ? TEXT("true") : TEXT("false"),
		Rate * Delta, PreviousCharge, CurrentCharge);*/

	if (IsValid(ChargeLight))
	{
		const float NewIntensity = MaxLightIntensity * CurrentCharge;
		ChargeLight->SetIntensity(NewIntensity);
		//LOG_Art(Log, TEXT("ALCLuxChargeTrigger::UpdateCharge ▶ 라이트 밝기 적용: %.1f"), NewIntensity);
	}

	if (!bChargeCompleted && CurrentCharge >= 1.0f)
	{
		bChargeCompleted = true;
		//LOG_Art(Log, TEXT("완전 충전 → 타겟 기믹 실행"));

		for (AActor* Target : LinkedTargets)
		{
			if (IsValid(Target) && Target->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
			{
				ILCGimmickInterface::Execute_ActivateGimmick(Target);
			}
		}
	}
	else if (bChargeCompleted && CurrentCharge <= 0.f)
	{
		bChargeCompleted = false;
		//LOG_Art(Log, TEXT("완전 방전 → 상태 초기화"));
	}
}
