#include "Framework/Manager/LCTimeManager.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include "LastCanary.h"

ALCTimeManager::ALCTimeManager()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	ElapsedTime = 0.0f;
	ElapsedDay = 1;
	CurrentPhase = EDayPhase::Day;
	CachedPhase = CurrentPhase;
}

void ALCTimeManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ElapsedTime = 0.0f;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_UpdateTime,
			this,
			&ALCTimeManager::UpdateTime,
			UpdateInterval,
			true
		);
	}
}

void ALCTimeManager::UpdateTime()
{
	ElapsedTime += UpdateInterval * TimeScale;
	CheckTimePhaseTransition();

	if (bEnableDebugLog)
	{
		FString Message = FString::Printf(TEXT("[TimeManager] Phase: %s | Elapsed: %.2f | Day: %d"),
			*UEnum::GetValueAsString(CurrentPhase),
			ElapsedTime,
			ElapsedDay);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Cyan,
				Message
			);
		}
	}

}

void ALCTimeManager::CheckTimePhaseTransition()
{
	float Limit = (CurrentPhase == EDayPhase::Day) ? DayDuration : NightDuration;

	if (ElapsedTime >= Limit)
	{
		float OverTime = ElapsedTime - Limit;
		if (CurrentPhase == EDayPhase::Night)
		{
			ElapsedDay++;
		}
		SetPhase(CurrentPhase == EDayPhase::Day ? EDayPhase::Night : EDayPhase::Day);
		ElapsedTime = OverTime;
	}

}


void ALCTimeManager::SetPhase(EDayPhase NewPhase)
{
	CurrentPhase = NewPhase;
	CachedPhase = NewPhase;

	if (bEnableDebugLog)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Yellow,
				FString::Printf(TEXT("[TimeManager] Phase Changed to %s"), *UEnum::GetValueAsString(NewPhase))
			);
		}
	}

	OnTimePhaseChanged.Broadcast(NewPhase);
}

void ALCTimeManager::OnRep_ElapsedTime()
{
	// 클라 UI용 처리 가능
}

void ALCTimeManager::OnRep_ElapsedDay()
{
	// 클라 UI용 처리 가능
}

void ALCTimeManager::OnRep_DayPhase()
{
	OnTimePhaseChanged.Broadcast(CurrentPhase);
}

void ALCTimeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALCTimeManager, ElapsedTime);
	DOREPLIFETIME(ALCTimeManager, ElapsedDay);
	DOREPLIFETIME(ALCTimeManager, CurrentPhase);
}