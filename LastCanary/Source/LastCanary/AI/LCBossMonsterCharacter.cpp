#include "AI/LCBossMonsterCharacter.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

ALCBossMonsterCharacter::ALCBossMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ALCBossMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurScale = MaxScale;
	SetActorScale3D(FVector(CurScale));
}

void ALCBossMonsterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority()) return;

	const bool bLooked = IsLookedAtByAnyPlayer();

	UpdateRage(DeltaSeconds, bLooked);
	UpdateScale(DeltaSeconds, bLooked);
	TryTriggerDarkness();
}

/* ───────── 시야 체크 ───────── */
bool ALCBossMonsterCharacter::IsLookedAtByAnyPlayer() const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (IsPlayerLooking(It->Get())) return true;
	return false;
}

bool ALCBossMonsterCharacter::IsPlayerLooking(APlayerController* PC) const
{
	if (!PC) return false;

	FVector ViewLoc; FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewLoc, ViewRot);

	const FVector Dir = (GetActorLocation() - ViewLoc).GetSafeNormal();
	const float Deg = FMath::RadiansToDegrees(acosf(FVector::DotProduct(ViewRot.Vector(), Dir)));
	return (Deg <= LookAngleDeg) && PC->LineOfSightTo(this, ViewLoc, true);
}

/* ───────── Rage & Scale ───────── */
void ALCBossMonsterCharacter::UpdateRage(float DeltaSeconds, bool bLooked)
{
	const float Delta = (bLooked ? -RageLossPerSec : RageGainPerSec) * DeltaSeconds;
	Rage = FMath::Clamp(Rage + Delta, 0.f, MaxRage);
}

void ALCBossMonsterCharacter::UpdateScale(float DeltaSeconds, bool bLooked)
{
	const float Target = bLooked ? MinScale : MaxScale;
	CurScale = FMath::FInterpTo(CurScale, Target, DeltaSeconds, ScaleInterpSpeed);
	SetActorScale3D(FVector(CurScale));
}

/* ───────── Darkness 스킬 ───────── */
void ALCBossMonsterCharacter::TryTriggerDarkness()
{
	if (bDarknessActive || Rage < DarknessRage - KINDA_SMALL_NUMBER) return;

	bDarknessActive = true;
	Multicast_StartDarkness();

	GetWorldTimerManager().SetTimer(DarknessTimer, this,
		&ALCBossMonsterCharacter::EndDarkness, DarknessDuration, false);
}

void ALCBossMonsterCharacter::EndDarkness()
{
	if (!bDarknessActive) return;
	bDarknessActive = false;
	Multicast_EndDarkness();
}

/* ───────── RepNotify ───────── */
void ALCBossMonsterCharacter::OnRep_Rage()
{
	// 예: 클라이언트 UI 업데이트
}

void ALCBossMonsterCharacter::OnRep_Scale()
{
	SetActorScale3D(FVector(CurScale));
}

/* ───────── Multicast ───────── */
void ALCBossMonsterCharacter::Multicast_StartDarkness_Implementation()
{
	BP_StartDarknessEffect();   // 파티클 대신 BP 후처리
}

void ALCBossMonsterCharacter::Multicast_EndDarkness_Implementation()
{
	BP_EndDarknessEffect();
}

/* ───────── NetReplication ───────── */
void ALCBossMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALCBossMonsterCharacter, Rage);
	DOREPLIFETIME(ALCBossMonsterCharacter, CurScale);
	DOREPLIFETIME(ALCBossMonsterCharacter, bDarknessActive);
}