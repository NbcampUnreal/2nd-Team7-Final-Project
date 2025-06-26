#include "Actor/Gimmick/Manager/LCTrackingManager.h"
#include "Actor/Gimmick/LCTrackingGimmick.h"
#include "AI/EliteMonster/TempleEliteMonster.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "LastCanary.h"

ALCTrackingManager::ALCTrackingManager()
	: DetectionDelay(2.0f)
	, TrackingDuration(20.0f)
	, FireOffset(10.0f)
	, CooldownDelay(3.0f)
	, bIsTrackingActive(false)
	, CurrentTarget(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALCTrackingManager::BeginPlay()
{
	Super::BeginPlay();
}

void ALCTrackingManager::RegisterPlayer(AActor* Player)
{
	if (!IsValid(Player)) return;

	if (!DetectedPlayers.Contains(Player))
	{
		DetectedPlayers.Add(Player);
		//LOG_Art(Log, TEXT(" 플레이어 등록됨: %s"), *Player->GetName());
	}

	if (!bIsTrackingActive && DetectedPlayers.Num() > 0)
	{
		GetWorldTimerManager().SetTimer(TrackingLoopHandle, this, &ALCTrackingManager::ChooseRandomTarget, DetectionDelay, false);
		bIsTrackingActive = true;
	}
}

void ALCTrackingManager::UnregisterPlayer(AActor* Player)
{
	if (DetectedPlayers.Remove(Player) > 0)
	{
		//LOG_Art(Log, TEXT(" 플레이어 제거됨: %s"), *Player->GetName());
	}

	if (DetectedPlayers.Num() == 0)
	{
		StopTrackingLoop();
	}
}

void ALCTrackingManager::RegisterGimmick(ALCTrackingGimmick* Gimmick)
{
	if (IsValid(Gimmick) && !TrackingTowers.Contains(Gimmick))
	{
		TrackingTowers.Add(Gimmick);
	}
}

void ALCTrackingManager::ChooseRandomTarget()
{
	if (DetectedPlayers.Num() == 0)
	{
		StopTrackingLoop();
		return;
	}

	TArray<AActor*> ValidTargets;
	for (TWeakObjectPtr<AActor> WeakPlayer : DetectedPlayers)
	{
		if (WeakPlayer.IsValid())
		{
			ValidTargets.Add(WeakPlayer.Get());
		}
	}

	if (ValidTargets.Num() == 0)
	{
		StopTrackingLoop();
		return;
	}

	int32 Index = UKismetMathLibrary::RandomInteger(ValidTargets.Num());
	CurrentTarget = ValidTargets[Index];

	//LOG_Art(Log, TEXT(" 타겟 선정: %s"), *CurrentTarget->GetName());

	UpdateAllTowers(CurrentTarget);

	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ALCTrackingManager::FireAllTowers, FireOffset, false);

	GetWorldTimerManager().SetTimer(TrackingStopTimerHandle, this, &ALCTrackingManager::StopTrackingLoop, TrackingDuration, false);
}

void ALCTrackingManager::UpdateAllTowers(AActor* Target)
{
	for (ALCTrackingGimmick* Tower : TrackingTowers)
	{
		if (IsValid(Tower))
		{
			Tower->SetTargetActor(Target);
		}
	}
}

void ALCTrackingManager::FireAllTowers()
{
	for (ALCTrackingGimmick* Tower : TrackingTowers)
	{
		if (IsValid(Tower))
		{
			Tower->Fire();
		}
	}

	//LOG_Art(Log, TEXT(" 모든 감시탑 이펙트 발사"));

	ATempleEliteMonster* bElite = Cast<ATempleEliteMonster>(this);
	if (IsValid(bElite))
	{
		bElite->ToggleOnReceive();
	}

	for (TActorIterator<ATempleEliteMonster> It(GetWorld()); It; ++It)
	{
		ATempleEliteMonster* Elite = *It;
		if (IsValid(Elite))
		{
			Elite->ReceiveTrackingTarget(CurrentTarget);
		}
	}
}

void ALCTrackingManager::StopTrackingLoop()
{
	bIsTrackingActive = false;
	CurrentTarget = nullptr;

	GetWorldTimerManager().ClearTimer(FireTimerHandle);
	GetWorldTimerManager().ClearTimer(TrackingStopTimerHandle);
	GetWorldTimerManager().ClearTimer(TrackingLoopHandle);

	for (ALCTrackingGimmick* Tower : TrackingTowers)
	{
		if (IsValid(Tower))
		{
			Tower->SetTargetActor(nullptr);
		}
	}

	ATempleEliteMonster* bElite = Cast<ATempleEliteMonster>(this);
	if (IsValid(bElite))
	{
		bElite->ToggleOnReceive();
	}

	//LOG_Art(Log, TEXT(" 추적 종료"));

	if (DetectedPlayers.Num() > 0)
	{
		GetWorldTimerManager().SetTimer(TrackingLoopHandle, this, &ALCTrackingManager::ChooseRandomTarget, CooldownDelay, false);
		bIsTrackingActive = true;
	}
}
