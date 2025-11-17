#include "AI/LCBossSlenderman.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineTypes.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "AI/LCBaseBossAIController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/BaseCharacter.h"

ALCBossSlenderman::ALCBossSlenderman()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ALCBossSlenderman::BeginPlay()
{
	Super::BeginPlay();

	// 시작 시 원래 속도 저장
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		DefaultWalkSpeed = MoveComp->MaxWalkSpeed;
	}

	GetWorldTimerManager().SetTimer(WhisperTimerHandle, this, &ALCBossSlenderman::ExecuteAbyssalWhisper, WhisperInterval, true);
	GetWorldTimerManager().SetTimer(TeleportTimerHandle, this, &ALCBossSlenderman::TeleportToRandomLocation, TeleportInterval, true);
	GetWorldTimerManager().SetTimer(DistortionTimerHandle, this, &ALCBossSlenderman::ExecuteDistortion, DistortionInterval, true);
	GetWorldTimerManager().SetTimer(EndlessStalkTimerHandle, this, &ALCBossSlenderman::EnterEndlessStalk, 60.f, false);
}

void ALCBossSlenderman::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 1) DeltaTime 유효성 검사
	if (DeltaTime <= KINDA_SMALL_NUMBER)
		return;

	// 2) 서버 권한 & World 널 체크
	UWorld* World = GetWorld();
	if (!World || !HasAuthority())
		return;

	// 3) Rage 갱신
	UpdateRage(DeltaTime);
}

void ALCBossSlenderman::UpdateBlackboardValues()
{
	Super::UpdateBlackboardValues();
}

bool ALCBossSlenderman::IsPlayerLookingAtMe(APawn* PlayerPawn) const
{
	// 0) 유효성 검사: PlayerPawn이 nullptr이거나 이미 파괴된 경우
	if (!IsValid(PlayerPawn))
	{
		return false;
	}

	// 1) 시야 회전 값 얻기: 컨트롤러가 있으면 그 회전, 없으면 Pawn의 회전 사용
	FRotator ViewRot;
	if (AController* C = PlayerPawn->GetController())
	{
		ViewRot = C->GetControlRotation();
	}
	else
	{
		ViewRot = PlayerPawn->GetActorRotation();
	}

	// 2) 플레이어 전방 벡터
	const FVector Forward = ViewRot.Vector();

	// 3) Boss까지의 방향 벡터
	const FVector ToBoss = GetActorLocation() - PlayerPawn->GetActorLocation();
	// 거의 같은 위치라면 false
	if (ToBoss.IsNearlyZero())
	{
		return false;
	}
	const FVector ToBossDir = ToBoss.GetSafeNormal();

	// 4) 내적 계산
	const float Dot = FVector::DotProduct(Forward, ToBossDir);

	// 5) 시야 한계치(예: 코사인 0.8 이상이면 정면으로 간주)
	return Dot > 0.8f;
}

void ALCBossSlenderman::UpdateRage(float DeltaSeconds)
{
	// 0) DeltaSeconds 유효성 검사
	if (DeltaSeconds <= KINDA_SMALL_NUMBER)
		return;

	// 1) 서버 권한 및 World 유효성 검사
	UWorld* World = GetWorld();
	if (!World || !HasAuthority())
		return;

	// 2) 부모 로직 (Decay 등) 먼저 실행
	Super::UpdateRage(DeltaSeconds);

	// 3) 기본 회복 적용
	float TotalDelta = BaseRagePerSecond * DeltaSeconds;

	// 4) 플레이어 시야 보정분 누적
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!IsValid(PC)) continue;

		APawn* P = PC->GetPawn();
		if (!IsValid(P)) continue;

		// (a) 플레이어 시야 방향
		FRotator ViewRot = PC->GetControlRotation();
		FVector Forward = ViewRot.Vector();

		// (b) 보스까지 방향
		FVector ToBoss = GetActorLocation() - P->GetActorLocation();
		if (ToBoss.IsNearlyZero()) continue;
		ToBoss.Normalize();

		// (c) Dot 체크
		const bool bLooking = FVector::DotProduct(Forward, ToBoss) > LookDotThreshold;

		// (d) 시야 분기: 시선 중이면 가속, 아니면 감속 보정
		TotalDelta += (bLooking ? LookRagePerSecond : -LoseRagePerSecond) * DeltaSeconds;
	}

	// 5) 한 번에 Rage 업데이트
	AddRage(TotalDelta);
}

void ALCBossSlenderman::AddRage(float Amount)
{
	if (!HasAuthority())
		return;

	// 0) MaxRage 유효성 검사
	if (MaxRage <= KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogTemp, Error, TEXT("[Slenderman] AddRage called with invalid MaxRage=%f"), MaxRage);
		return;
	}

	// 1) Rage 갱신 및 클램프, 블랙보드값 업데이트
	Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);

	UpdateBlackboardValues();

	if (Rage >= MaxRage && !bIsBerserk)
	{
		// Berserk 시작 (Duration이 0 이하일 수 있으니 검사)
		if (BerserkDuration > 0.f)
		{
			StartBerserk(BerserkDuration);
		}

		UpdateBlackboardValues();
	}

	// Rage 일정치값 이상이면 FearWave실행(특수기)
	if (Rage >= FearWaveRageThreshold)
	{
		ExecuteFearWave();
	}
}

void ALCBossSlenderman::EnterBerserkState()
{
	Super::EnterBerserkState();
	UE_LOG(LogTemp, Warning, TEXT("[Slenderman] 영구 Berserk 진입"));
}

void ALCBossSlenderman::StartBerserk()
{
	Super::StartBerserk();  // bIsBerserk = true 및 Multicast 호출

	EnterEndlessStalk();

	if (ShadowCloneClass)
	{
		for (int32 i = 0; i < BerserkCloneCount; ++i)
		{
			FVector Offset = FMath::VRand() * 300.f;
			FActorSpawnParameters Params;
			Params.Owner = this;
			if (AActor* Clone = GetWorld()->SpawnActor<AActor>(
				ShadowCloneClass,
				GetActorLocation() + Offset,
				GetActorRotation(),
				Params))
			{
				ShadowClones.Add(Clone);
			}
		}
	}
}

void ALCBossSlenderman::StartBerserk(float Duration)
{
	// 서버: 먼저 무한 버전 실행
	Super::StartBerserk(Duration);

	EnterEndlessStalk();

	if (ShadowCloneClass)
	{
		for (int32 i = 0; i < BerserkCloneCount; ++i)
		{
			FVector Offset = FMath::VRand() * 300.f;
			FActorSpawnParameters Params;
			Params.Owner = this;
			if (AActor* Clone = GetWorld()->SpawnActor<AActor>(
				ShadowCloneClass,
				GetActorLocation() + Offset,
				GetActorRotation(),
				Params))
			{
				ShadowClones.Add(Clone);
			}
		}
	}
}

void ALCBossSlenderman::EndBerserk()
{
	Super::EndBerserk();
	UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Berserk 종료"));

	// (서버) Speed 원복 및 분신 정리
	for (AActor* Clone : ShadowClones)
	{
		if (IsValid(Clone))
		{
			Clone->Destroy();
		}
	}
	ShadowClones.Empty();

	// (서버) EndlessStalk 종료
	if (HasAuthority() && bIsEndlessStalk)
	{
		bIsEndlessStalk = false;
		// 즉시 로컬 서버에서도 복원
		OnRep_EndlessStalk();
	}
}

void ALCBossSlenderman::ExecuteFearWave()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
		return;

	// 1) 시각·음향 효과 (멀티캐스트)
	Multicast_PlayFearWaveEffects();

	// 2) 맵상의 모든 플레이어에게 데미지 및 공포·슬로우
	const float DamageAmount = FearWaveDamage;      // UPROPERTY 노출된 값
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!IsValid(PC))
			continue;

		APawn* Pawn = PC->GetPawn();
		if (!IsValid(Pawn))
			continue;

		auto* Target = Cast<ABaseCharacter>(Pawn);
		if (!Target)
			continue;

		// ── 2‑1) 일반 대미지 적용
		UGameplayStatics::ApplyDamage(
			Target,
			DamageAmount,
			GetController(),
			this,
			UDamageType::StaticClass()
		);

		// ── 2‑2) 공포 디버프 (포스트프로세스)
		Client_ApplyFearPostProcess(Target);

		// ── 2‑3) 슬로우 디버프
		if (UCharacterMovementComponent* MoveComp = Target->GetCharacterMovement())
		{
			const float OrigSpeed = MoveComp->MaxWalkSpeed;
			MoveComp->MaxWalkSpeed = OrigSpeed * FearSlowMultiplier;

			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(
				TimerHandle,
				FTimerDelegate::CreateLambda([MoveComp, OrigSpeed]()
					{
						if (IsValid(MoveComp))
							MoveComp->MaxWalkSpeed = OrigSpeed;
					}),
				FearSlowDuration,
				false
			);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Slenderman] ExecuteFearWave: applied to all players"));
}

void ALCBossSlenderman::Multicast_PlayFearWaveEffects_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	FVector Loc = GetActorLocation();

	// VFX
	if (FearWaveFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			FearWaveFX,
			Loc,
			FRotator::ZeroRotator,
			FVector(1.f),
			true
		);
	}

	// SFX with attenuation
	if (FearWaveSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FearWaveSound,
			Loc
		);
	}
}

void ALCBossSlenderman::Client_ApplyFearPostProcess_Implementation(ACharacter* Target)
{
	// Only on clients
	if (!FearPostProcessMaterial)
		return;

	// Find local pawn's camera
	if (APawn* P = Cast<APawn>(GetOwner()))
	{
		if (UCameraComponent* Cam = P->FindComponentByClass<UCameraComponent>())
		{
			Cam->PostProcessSettings.AddBlendable(FearPostProcessMaterial, FearPPBlendWeight);

			// remove after duration
			FTimerHandle Handle;
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					Handle,
					FTimerDelegate::CreateLambda([Cam, this]()
						{
							if (Cam && FearPostProcessMaterial)
							{
								Cam->PostProcessSettings.RemoveBlendable(FearPostProcessMaterial);
							}
						}),
					FearPPDuration,
					false
				);
			}
		}
	}
}

void ALCBossSlenderman::ExecuteAbyssalWhisper()
{
	// 서버 전용 & World 유효성 검사
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 플레이어마다 위치 왜곡 후 멀티캐스트 호출
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!IsValid(PC))
			continue;

		APawn* Pawn = PC->GetPawn();
		if (!IsValid(Pawn))
			continue;

		const FVector TrueLoc = Pawn->GetActorLocation();
		const FVector FakeLoc = TrueLoc + FMath::VRand() * 500.f;  // UPROPERTY에 선언된 반경 사용

		// 클라이언트에 재생 요청
		Multicast_PlayAbyssalWhisper(FakeLoc);

		UE_LOG(LogTemp, Warning,
			TEXT("[Slenderman] Whisper distorted for %s at %s"),
			*PC->GetName(),
			*FakeLoc.ToString());
	}
}

void ALCBossSlenderman::Multicast_PlayAbyssalWhisper_Implementation(const FVector& Location)
{
	// 클라이언트 전용 & 사운드 유효성 검사
	if (!IsValid(WhisperSound))
		return;

	// Attenuation이 설정되어 있으면 적용, 없으면 nullptr로 전역 사운드처럼 재생
	UGameplayStatics::PlaySoundAtLocation(
		this,
		WhisperSound,
		Location,
		/*VolumeMultiplier=*/1.f,
		/*PitchMultiplier=*/1.f,
		/*StartTime=*/0.f,
		WhisperAttenuation  // ← 여기에 Attenuation 에셋 지정
	);
}

void ALCBossSlenderman::TeleportToRandomLocation()
{
	// 0) 서버 전용 검사
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 네비게이션 시스템 & 랜덤 포인트
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys)
		return;

	FNavLocation NavLoc;
	constexpr float Radius = 1000.f;
	if (!NavSys->GetRandomPointInNavigableRadius(GetActorLocation(), Radius, NavLoc))
		return;

	// 2) 캡슐 컴포넌트 얻기
	UCapsuleComponent* Cap = FindComponentByClass<UCapsuleComponent>();
	float CapsuleHalfHeight = Cap ? Cap->GetScaledCapsuleHalfHeight() : 0.f;
	float CapsuleRadius = Cap ? Cap->GetScaledCapsuleRadius() : 50.f;

	// 3) 바닥 높이 보정 (라인 트레이스)
	FVector Desired = NavLoc.Location;
	{
		const float TraceUp = 500.f;
		const float TraceDown = 1000.f;
		FVector Start = Desired + FVector(0, 0, TraceUp);
		FVector End = Desired - FVector(0, 0, TraceDown);

		FHitResult Hit;
		FCollisionQueryParams Params(NAME_None, false, this);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			Desired.Z = Hit.Location.Z + CapsuleHalfHeight;
		}
		else
		{
			Desired.Z += CapsuleHalfHeight;
		}
	}

	// 4) 충돌 스윕 체크: 벽이나 장애물 안에 들어가지 않도록
	FHitResult SweepHit;
	FCollisionQueryParams SweepParams(NAME_None, false, this);
	if (World->SweepSingleByChannel(
		SweepHit,
		GetActorLocation(),
		Desired,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
		SweepParams))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Teleport path blocked, aborting"));
		return;
	}

	// 5) 텔레포트
	SetActorLocation(Desired, /*bSweep=*/false, /*OutSweepHitResult=*/nullptr, ETeleportType::TeleportPhysics);

	// 6) 모든 클라이언트에 VFX/SFX 요청
	Multicast_PlayTeleportEffects(Desired);
}

void ALCBossSlenderman::Multicast_PlayTeleportEffects_Implementation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// VFX
	if (IsValid(TeleportFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			TeleportFX,
			Location,
			FRotator::ZeroRotator,
			FVector(1.f),
			true
		);
	}

	// SFX with attenuation
	if (IsValid(TeleportSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			TeleportSound,
			Location,
			/*VolumeMultiplier=*/1.f,
			/*PitchMultiplier=*/1.f,
			/*StartTime=*/0.f,
			TeleportAttenuation  // attenuation asset or nullptr
		);
	}
}

void ALCBossSlenderman::ExecuteDistortion()
{
	// (1) 서버 전용 확인
	if (!HasAuthority())
		return;

	// (2) World 유효성 검사
	UWorld* World = GetWorld();
	if (!World)
		return;

	// (3) 모든 클라이언트에게 이펙트/SFX 재생 요청
	Multicast_DistortionEffect();
}

void ALCBossSlenderman::Multicast_DistortionEffect_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Distortion Effect Triggered"));

	UWorld* World = GetWorld();
	if (!World)
		return;

	const FVector Loc = GetActorLocation();

	// (0) VFX
	if (IsValid(DistortionFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			DistortionFX,
			Loc,
			FRotator::ZeroRotator,
			FVector(1.f),
			true
		);
	}

	// (1) SFX with attenuation
	if (IsValid(DistortionSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DistortionSound,
			Loc
		);
	}

	// (2) BP/머티리얼 맵 왜곡 처리…
	// 필요한 경우 여기서도 World·Asset 유효성 검사 추가
}

void ALCBossSlenderman::EnterEndlessStalk()
{
	// 서버 전용 & 이미 진입했는지 확인
	if (!HasAuthority() || bIsEndlessStalk)
		return;

	// World 검사
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 플래그 설정
	bIsEndlessStalk = true;

	// 클라이언트에 효과 시작 알림
	Multicast_StartEndlessStalk();
}

void ALCBossSlenderman::OnRep_EndlessStalk()
{
	// CharacterMovementComponent 유효성 검사
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (bIsEndlessStalk)
		{
			// EndlessStalk 시작: 속도 2배
			MoveComp->MaxWalkSpeed = DefaultWalkSpeed * 2.f;
			UE_LOG(LogTemp, Warning, TEXT("[Slenderman] EndlessStalk ON: WalkSpeed=%.1f"), MoveComp->MaxWalkSpeed);
		}
		else
		{
			// EndlessStalk 종료: 원래 속도로 복원
			MoveComp->MaxWalkSpeed = DefaultWalkSpeed;
			UE_LOG(LogTemp, Warning, TEXT("[Slenderman] EndlessStalk OFF: WalkSpeed=%.1f"), MoveComp->MaxWalkSpeed);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Slenderman] OnRep_EndlessStalk: CharacterMovementComponent 없음"));
	}
}

void ALCBossSlenderman::Multicast_StartEndlessStalk_Implementation()
{
	// (0) VFX
	if (EndlessStalkFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			EndlessStalkFX,
			GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
	// (1) SFX
	if (EndlessStalkSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EndlessStalkSound,
			GetActorLocation()
		);
	}

	OnRep_EndlessStalk();
	UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Endless Stalk Activated"));
}

void ALCBossSlenderman::ExecuteReachSlash()
{
	// 서버 전용 & World 체크
	if (!HasAuthority())
		return;
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 쿨다운 타이머 설정
	World->GetTimerManager().SetTimer(ReachSlashTimerHandle, ReachSlashCooldown, false);

	const FVector Origin = GetActorLocation();

	// (1) 데미지 판정
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ReachSlashRadius);
	if (World->SweepMultiByChannel(Hits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere))
	{
		for (const FHitResult& H : Hits)
		{
			if (ABaseCharacter* C = Cast<ABaseCharacter>(H.GetActor()))
			{
				UGameplayStatics::ApplyDamage(
					C,
					ReachSlashDamage,
					GetController(),
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}

	// (2) 모든 클라이언트에 VFX/SFX 재생 요청
	Multicast_PlayReachSlashEffects(Origin);
}

void ALCBossSlenderman::Multicast_PlayReachSlashEffects_Implementation(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// VFX
	if (IsValid(ReachSlashFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ReachSlashFX,
			Origin,
			FRotator::ZeroRotator,
			FVector(1.f),
			true
		);
	}

	// SFX with attenuation
	if (IsValid(ReachSlashSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ReachSlashSound,
			Origin,
			/*VolumeMultiplier=*/1.f,
			/*PitchMultiplier=*/1.f,
			/*StartTime=*/0.f,
			AttackAttenuation
		);
	}
}

void ALCBossSlenderman::ExecuteShadowGrasp()
{
	// 서버 전용 & World 체크
	if (!HasAuthority())
		return;
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 쿨다운 타이머 설정
	World->GetTimerManager().SetTimer(ShadowGraspTimerHandle, ShadowGraspCooldown, false);

	// 타겟 획득
	if (AAIController* AC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AC->GetBlackboardComponent())
		{
			UObject* Obj = BB->GetValueAsObject(TEXT("TargetActor"));
			ABaseCharacter* Target = Obj ? Cast<ABaseCharacter>(Obj) : nullptr;
			if (IsValid(Target))
			{
				// 넉백
				FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				Target->LaunchCharacter(-Dir * 1000.f, true, true);

				// 모든 클라이언트에 VFX/SFX 재생 요청
				Multicast_PlayShadowGraspEffects(Target->GetActorLocation());
			}
		}
	}
}

void ALCBossSlenderman::Multicast_PlayShadowGraspEffects_Implementation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// VFX
	if (IsValid(ShadowGraspFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ShadowGraspFX,
			Location,
			FRotator::ZeroRotator,
			FVector(1.f),
			true
		);
	}

	// SFX with attenuation
	if (IsValid(ShadowGraspSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ShadowGraspSound,
			Location,
			/*VolumeMultiplier=*/1.f,
			/*PitchMultiplier=*/1.f,
			/*StartTime=*/0.f,
			AttackAttenuation
		);
	}
}

void ALCBossSlenderman::ExecuteAttackDistortion()
{
	// (1) 서버 전용 검사
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// (2) 쿨다운 타이머
	World->GetTimerManager().SetTimer(
		AttackDistortionTimerHandle,
		DistortionCooldown,
		false
	);

	UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Attack Distortion Triggered"));

	// (3) 범위 내 ABaseCharacter 수집
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(DistortionRadius);
	if (World->SweepMultiByChannel(
		Hits,
		GetActorLocation(), GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere))
	{
		for (const FHitResult& H : Hits)
		{
			if (auto* C = Cast<ABaseCharacter>(H.GetActor()))
			{
				if (IsValid(C))
					AffectedPlayers.AddUnique(C);
			}
		}
	}

	// (4) **클라이언트에 VFX/SFX 재생 요청** (보스 위치 기준)
	Multicast_PlayAttackDistortionEffects(GetActorLocation());

	FVector BossLoc = GetActorLocation();

	// (5) 플레이어들만 랜덤 위치로 텔레포트
	for (auto* C : AffectedPlayers)
	{
		if (!IsValid(C))
			continue;

		// (5‑1) 랜덤 방향 & 위치 계산 (XY 평면)
		FVector Dir = FMath::VRand();
		Dir.Z = 0.f;
		Dir = Dir.GetSafeNormal();
		if (Dir.IsNearlyZero())
			continue;

		FVector Desired = BossLoc + Dir * AttackDistortionRange;

		// (5‑2) 내비게이션 메쉬 보정
		if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			FNavLocation Projected;
			if (NavSys->ProjectPointToNavigation(Desired, Projected, FVector(200.f)))
			{
				Desired = Projected.Location;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Player Distortion NavMesh 보정 실패, 해당 플레이어 건너뜀"));
				continue;
			}
		}

		// (5‑3) 플레이어 텔레포트 (Sweep 충돌처리)
		C->SetActorLocation(Desired, /*bSweep=*/true, /*OutSweepHitResult=*/nullptr, ETeleportType::TeleportPhysics);
	}

	// (6) 배열 비우기
	AffectedPlayers.Empty();
}

void ALCBossSlenderman::Multicast_PlayAttackDistortionEffects_Implementation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// (a) VFX
	if (AttackDistortionFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			AttackDistortionFX,
			Location,
			FRotator::ZeroRotator,
			FVector(1.f),
			true
		);
	}

	// (b) SFX with attenuation
	if (AttackDistortionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			AttackDistortionSound,
			Location,
			1.f,                      // VolumeMultiplier
			1.f,                      // PitchMultiplier
			0.f,                      // StartTime
			AttackAttenuation
		);
	}
}

bool ALCBossSlenderman::RequestAttack(float TargetDistance)
{
	// 0) 서버 권한 및 World 유효성 검사
	if (!HasAuthority())
		return false;

	UWorld* World = GetWorld();
	if (!World)
		return false;

	// 1) 컨트롤러 & AIController & Blackboard 유효성 검사
	AController* C = GetController();
	if (!C)
		return false;

	AAIController* AICon = Cast<AAIController>(C);
	if (!AICon)
		return false;

	UBlackboardComponent* BB = AICon->GetBlackboardComponent();
	if (!BB)
		return false;

	// 2) 타겟 획득 (Blackboard에 Object로 저장됨)
	UObject* Obj = BB->GetValueAsObject(TEXT("TargetActor"));
	AActor* Target = Cast<AActor>(Obj);
	const bool bHasTarget = IsValid(Target);

	// 3) 현재 시간
	const float Now = World->GetTimeSeconds();

	// 4) 쿨타임 기준으로 후보군만 수집 (거리 검사 제거)
	struct FEntry { float Weight, Range; TFunction<void()> Action; };
	TArray<FEntry> Entries;

	// ReachSlash
	if (Now - LastReachSlashTime >= ReachSlashCooldown)
	{
		Entries.Add({
			3.f,                     // 가중치
			ReachSlashRadius,        // 사거리 정보만 기록
			[this, Now]() {
				LastReachSlashTime = Now;
				ExecuteReachSlash();
			}
			});
	}

	// ShadowGrasp 후보
	if (Now - LastShadowGraspTime >= ShadowGraspCooldown)
	{
		Entries.Add({
			2.f,
			ShadowGraspDistance,
			[this, Now]() {
				LastShadowGraspTime = Now;
				ExecuteShadowGrasp();
			}
			});
	}

	// AttackDistortion 후보
	if (Now - LastAttackDistortionTime >= DistortionCooldown)
	{
		Entries.Add({
			1.f,
			AttackDistortionRange,
			[this, Now]() {
				LastAttackDistortionTime = Now;
				ExecuteAttackDistortion();
			}
			});
	}

	
	if (Entries.IsEmpty())
		return false;

	// 5) 가중치 랜덤 선택
	float TotalW = 0.f;
	for (auto& E : Entries) TotalW += E.Weight;

	float Pick = FMath::FRandRange(0.f, TotalW), Acc = 0.f;
	for (auto& E : Entries)
	{
		Acc += E.Weight;
		if (Pick <= Acc)
		{
			NextAttackRange = E.Range;
			NextAttackAction = E.Action;
			return true;
		}
	}

	return false;
}

void ALCBossSlenderman::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCBossSlenderman, bIsEndlessStalk);
}