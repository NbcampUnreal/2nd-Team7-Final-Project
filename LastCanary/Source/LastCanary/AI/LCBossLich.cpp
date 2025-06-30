#include "AI/LCBossLich.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/BaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "AI/Projectile/ArcaneBolt.h"
#include "AI/LCBaseBossAIController.h"
#include "Components/CapsuleComponent.h"

ALCBossLich::ALCBossLich()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ALCBossLich::BeginPlay()
{
	Super::BeginPlay();


	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[LCBossLich::BeginPlay] World is null!"));
		return;
	}

	// (1) 서버 권한이 있고, 소환 클래스 배열이 비어있지 않을 때만 타이머 등록
	if (HasAuthority() && UndeadClasses.Num() > 0 && UndeadSpawnInterval > 0.f)
	{
		World->GetTimerManager().SetTimer(
			UndeadSpawnTimerHandle,
			this,
			&ALCBossLich::SpawnUndeadMinion,
			UndeadSpawnInterval,
			true
		);
	}

	// (2) ManaPulseInterval이 유효할 때만 타이머 등록
	if (ManaPulseInterval > 0.f)
	{
		World->GetTimerManager().SetTimer(
			ManaPulseTimerHandle,
			this,
			&ALCBossLich::ExecuteManaPulse,
			ManaPulseInterval,
			true
		);
	}
}

void ALCBossLich::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
		return;

	UpdateRage(DeltaTime);
}

void ALCBossLich::UpdateBlackboardValues()
{
	Super::UpdateBlackboardValues();
}

void ALCBossLich::UpdateRage(float DeltaSeconds)
{
	// 0) DeltaSeconds 유효성 검사
	if (DeltaSeconds <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LCBossLich::UpdateRage] Invalid DeltaSeconds: %f"), DeltaSeconds);
		return;
	}

	// 1) 부모 로직 호출
	Super::UpdateRage(DeltaSeconds);

	// 2) 잘못된(죽거나 파괴된) 언데드는 목록에서 제거
	SpawnedUndeadMinions.RemoveAll([](AActor* Minion) {
		return !IsValid(Minion);
		});

	// 3) 살아있는 언데드 수 집계
	const int32 AliveCount = SpawnedUndeadMinions.Num();

	// 4) DeltaRage 계산
	float DeltaRage = 0.f;
	if (AliveCount > 0)
	{
		// 살아있는 언데드가 있으면 증가
		DeltaRage = UndeadRagePerSecond * AliveCount * DeltaSeconds;
	}
	else
	{
		// 하나도 없으면 감소
		DeltaRage = -UndeadRagePerSecond * DeltaSeconds;
	}
	AddRage(DeltaRage);

	if (HasAuthority() && !bHasUsedPhantomVortex && Rage >= PhantomVortexRageThreshold)
	{
		bHasUsedPhantomVortex = true;
		PhantomVortex();
	}
}

void ALCBossLich::AddRage(float Amount)
{
	if (!HasAuthority())
		return;

	// 0) MaxRage 유효성 검사
	if (MaxRage <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[Lich::AddRage] MaxRage 값이 유효하지 않습니다: %f"), MaxRage);
		return;
	}

	// 1) Rage 누적 및 클램프
	Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);

	// 2) 서버 권한이 있을 때만 Blackboard 업데이트

	UpdateBlackboardValues();


	// 3) 광폭화 진입 조건 확인 (서버 전용)
	if (Rage >= MaxRage && !bIsBerserk)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Lich] Rage가 최대치에 도달하여 Berserk 모드 진입"));
		StartBerserk(BerserkDuration);

		// EnterBerserkState() 내부에서도 HasAuthority 체크가 이루어지지만,
		// 필요하다면 즉시 Blackboard 갱신
		UpdateBlackboardValues();
	}
}


// ── 영구 Berserk 진입 시 로그만 ─────────────────
void ALCBossLich::EnterBerserkState()
{
	Super::EnterBerserkState();
	UE_LOG(LogTemp, Warning, TEXT("[Lich] 영구 Berserk 진입"));
}

// ── 서버 로직 수행 후 멀티캐스트 포함 ─────────────────
void ALCBossLich::StartBerserk()
{
	Super::StartBerserk();

	// 저장
	PrevArcaneBoltCooldown = ArcaneBoltCooldown;
	PrevSoulBindCooldown = SoulBindCooldown;
	PrevDeathNovaDamage = DeathNovaDamage;
	PrevSoulAbsorbDamage = SoulAbsorbDamage;

	// 쿨다운 50% 감소
	ArcaneBoltCooldown *= BerserkCooldownFactor;
	SoulBindCooldown *= BerserkCooldownFactor;

	// 데미지 30% 증가
	DeathNovaDamage *= BerserkDamageFactor;
	SoulAbsorbDamage *= BerserkDamageFactor;

}

// ── 지속시간 지정 버전 (StartBerserk(float)) ─────────────────
void ALCBossLich::StartBerserk(float Duration)
{
	Super::StartBerserk(Duration);

	// 저장
	PrevArcaneBoltCooldown = ArcaneBoltCooldown;
	PrevSoulBindCooldown = SoulBindCooldown;
	PrevDeathNovaDamage = DeathNovaDamage;
	PrevSoulAbsorbDamage = SoulAbsorbDamage;

	// 쿨다운 50% 감소
	ArcaneBoltCooldown *= BerserkCooldownFactor;
	SoulBindCooldown *= BerserkCooldownFactor;

	// 데미지 30% 증가
	DeathNovaDamage *= BerserkDamageFactor;
	SoulAbsorbDamage *= BerserkDamageFactor;

}

// ── Berserk 종료 시 로그만 (후처리 필요 시 여기에 추가) ─────────────────
void ALCBossLich::EndBerserk()
{
	Super::EndBerserk();

	// 원래 값으로 복구
	ArcaneBoltCooldown = PrevArcaneBoltCooldown;
	SoulBindCooldown = PrevSoulBindCooldown;
	DeathNovaDamage = PrevDeathNovaDamage;
	SoulAbsorbDamage = PrevSoulAbsorbDamage;
}

void ALCBossLich::SpawnUndeadMinion()
{
	// 0) 서버 권한 & 월드 유효성 검사
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
		return;

	// 1) 소환 클래스 배열 & 최대 언데드 수 확인
	if (UndeadClasses.Num() == 0 || MaxUndeadMinions <= 0)
		return;

	// 2) 기존에 죽은 언데드는 리스트에서 제거
	SpawnedUndeadMinions.RemoveAll([](APawn* P) {
		return !IsValid(P);
		});

	// 3) 현재 살아있는 언데드 수가 최대치를 넘으면 중단
	if (SpawnedUndeadMinions.Num() >= MaxUndeadMinions)
		return;

	// 4) 랜덤 클래스 선택 안전하게
	int32 ClassCount = UndeadClasses.Num();
	if (ClassCount <= 0)
		return;

	int32 idx = FMath::RandRange(0, ClassCount - 1);
	TSubclassOf<APawn> Cls = UndeadClasses[idx];
	if (!Cls)
		return;

	// 5) 안전한 위치 계산
	FVector Origin = GetActorLocation();
	FVector MinOffset(-500.f, -500.f, 0.f), MaxOffset(500.f, 500.f, 0.f);
	FVector Loc = Origin + FMath::RandPointInBox(FBox(MinOffset, MaxOffset));

	// 6) 스폰 파라미터
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = Cast<APawn>(GetController() ? GetController()->GetPawn() : nullptr);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 7) 실제 소환
	APawn* M = World->SpawnActor<APawn>(Cls, Loc, FRotator::ZeroRotator, Params);
	if (!IsValid(M))
		return;

	// 8) 리스트에 추가 & 콜백 바인딩
	SpawnedUndeadMinions.Add(M);
	M->OnDestroyed.AddDynamic(this, &ALCBossLich::OnUndeadDestroyed);

	// 9) 즉시 Rage 상승
	AddRage(UndeadRagePerSecond);
}

void ALCBossLich::OnUndeadDestroyed(AActor* DestroyedActor)
{
	// 0) 서버 권한 및 파라미터 유효성 검사
	if (!HasAuthority() || !DestroyedActor)
		return;

	// 1) Pawn으로 캐스트
	APawn* Pawn = Cast<APawn>(DestroyedActor);
	if (!Pawn)
		return;

	// 2) 배열에서 잘못된(이미 파괴된) 엔트리 정리
	SpawnedUndeadMinions.RemoveAll([](APawn* P) {
		return !IsValid(P);
		});

	// 3) 실제로 배열에 남아 있던 Pawn인지 확인하며 제거
	if (SpawnedUndeadMinions.Remove(Pawn) > 0)
	{
		// 4) 페널티 Rage 적용 (Penalty 값 유효성 검사)
		if (UndeadDeathRagePenalty > 0.f)
		{
			AddRage(-UndeadDeathRagePenalty);
		}

		// 5) 바인딩 해제 
		Pawn->OnDestroyed.RemoveDynamic(this, &ALCBossLich::OnUndeadDestroyed);
	}
}

// Phantom Vortex 실행
void ALCBossLich::PhantomVortex()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Lich] PhantomVortex 발동"));

	// 1) 모든 클라이언트에 FX/SFX 재생
	Multicast_PlayPhantomVortexEffects();

	// 2) 모든 캐릭터 스턴
	{
		TArray<AActor*> AllChars;
		UGameplayStatics::GetAllActorsOfClass(World, ABaseCharacter::StaticClass(), AllChars);
		for (AActor* Actor : AllChars)
		{
			if (Actor == this) continue;
			if (auto* C = Cast<ABaseCharacter>(Actor))
			{
				if (auto* Move = C->GetCharacterMovement())
					Move->DisableMovement();
			}
		}
	}

	// 3) Tick 데미지 타이머 등록
	if (PhantomVortexTickInterval > 0.f)
	{
		World->GetTimerManager().SetTimer(
			PhantomVortexTickHandle,
			this, &ALCBossLich::TickPhantomVortexDamage,
			PhantomVortexTickInterval,
			true
		);
	}

	// 4) 지속시간 후 → Tick 중지 + 스턴 해제
	if (PhantomVortexDuration > 0.f)
	{
		FTimerHandle EndHandle;
		World->GetTimerManager().SetTimer(
			EndHandle,
			FTimerDelegate::CreateLambda([this]()
				{
					UWorld* InnerWorld = GetWorld();
					if (!InnerWorld) return;

					// (a) Tick 타이머 정리
					InnerWorld->GetTimerManager().ClearTimer(PhantomVortexTickHandle);

					// (b) 모든 캐릭터 스턴 해제
					TArray<AActor*> AllChars2;
					UGameplayStatics::GetAllActorsOfClass(
						InnerWorld,
						ABaseCharacter::StaticClass(),
						AllChars2
					);

					for (AActor* Actor : AllChars2)
					{
						if (Actor == this) continue;
						if (auto* C = Cast<ABaseCharacter>(Actor))
						{
							if (auto* Move = C->GetCharacterMovement())
								Move->SetMovementMode(MOVE_Walking);
						}
					}
				}),
			PhantomVortexDuration,
			false
		);
	}
}

void ALCBossLich::Multicast_PlayPhantomVortexEffects_Implementation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// FX
	if (PhantomVortexFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			PhantomVortexFX,
			GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// SFX: PlaySoundAtLocation so attenuation applies on each client
	if (PhantomVortexSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PhantomVortexSound,
			GetActorLocation()
		);
	}
}

// Phantom Vortex 범위 데미지 처리
void ALCBossLich::TickPhantomVortexDamage()
{
	// (0) 서버 권한 및 World 유효성 검사
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// (1) DamagePerTick 유효성 검사
	if (PhantomVortexDamagePerTick <= 0.f)
		return;

	UE_LOG(LogTemp, Verbose, TEXT("[Lich] PhantomVortex 전역 Tick 데미지 실행"));

	// (2) 대상 수집
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(
		World,
		ABaseCharacter::StaticClass(),
		AllActors
	);

	// (3) 대미지 적용
	AController* InstigatorController = GetController();
	for (AActor* Actor : AllActors)
	{
		if (!Actor || Actor == this)
			continue;

		if (auto* C = Cast<ABaseCharacter>(Actor))
		{
			UGameplayStatics::ApplyDamage(
				C,
				PhantomVortexDamagePerTick,
				InstigatorController,
				this,
				nullptr
			);
		}
	}
}

void ALCBossLich::ExecuteManaPulse()
{
	// 0) 서버 권한 및 World 유효성 검사
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 파라미터 유효성 검사
	if (ManaPulseRadius <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteManaPulse: 잘못된 Radius (%.1f)"), ManaPulseRadius);
		return;
	}
	if (ManaPulseDamage <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteManaPulse: 잘못된 Damage (%.1f)"), ManaPulseDamage);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteManaPulse 시작: Damage=%.1f, Radius=%.1f"), ManaPulseDamage, ManaPulseRadius);

	FVector Origin = GetActorLocation();

	// 2) 디버그 스피어 (World null 체크 완료)
	DrawDebugSphere(
		World,
		Origin,
		ManaPulseRadius,
		16,
		FColor::Blue,
		false,
		2.0f,
		0,
		5.0f
	);

	// 3) SweepMulti를 위한 준비
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ManaPulseRadius);

	// 4) 충돌 검사
	if (World->SweepMultiByChannel(Hits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere))
	{
		AController* InstigatorController = GetController();  // null 체크는 ApplyDamage 내부에서 처리됨

		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActor == this)
				continue;

			if (auto* C = Cast<ABaseCharacter>(HitActor))
			{
				UE_LOG(LogTemp, Warning, TEXT("[Lich] ManaPulse → %s에 %.1f 대미지"), *C->GetName(), ManaPulseDamage);
				UGameplayStatics::ApplyDamage(
					C,
					ManaPulseDamage,
					InstigatorController,
					this,
					nullptr
				);
			}
		}
	}
}

void ALCBossLich::ExecuteArcaneBolt(AActor* Target)
{
	// 0) 서버 권한 및 필수 포인터 유효성 검사
	if (!HasAuthority() || !Target || !ArcaneBoltClass)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	AController* InstigatorController = GetController();
	APawn* InstigatorPawn = InstigatorController ? Cast<APawn>(InstigatorController->GetPawn()) : nullptr;

	UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteArcaneBolt 시작 → 대상: %s"), *Target->GetName());

	// 1) 소환 위치: 본 액터의 콜리전 캡슐 앞쪽 바로 외부
	FVector SpawnLoc = GetActorLocation();
	if (UCapsuleComponent* Capsule = FindComponentByClass<UCapsuleComponent>())
	{
		float Offset = Capsule->GetScaledCapsuleHalfHeight() + 50.f;
		SpawnLoc += GetActorForwardVector() * Offset;
	}
	// 2) 회전: 타겟을 향하도록
	FVector ToTarget = Target->GetActorLocation() - SpawnLoc;
	if (ToTarget.IsNearlyZero())
		ToTarget = GetActorForwardVector();
	FRotator SpawnRot = ToTarget.Rotation();
	FVector Dir = SpawnRot.Vector();

	// 3) 스폰 파라미터
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = InstigatorPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 4) 스폰
	AArcaneBolt* Bolt = World->SpawnActor<AArcaneBolt>(
		ArcaneBoltClass,
		SpawnLoc,
		SpawnRot,
		Params
	);
	if (!Bolt)
		return;

	// 5) 속도·데미지 계산
	float Speed = bIsBerserk
		? ArcaneBoltSpeed * BerserkCooldownFactor
		: ArcaneBoltSpeed;
	float Damage = bIsBerserk
		? NormalAttackDamage * BerserkDamageFactor
		: NormalAttackDamage;

	UE_LOG(LogTemp, Warning, TEXT("[Lich] ArcaneBolt 스폰 → Speed=%.1f, Damage=%.1f"), Speed, Damage);

	// 6) 초기화
	Bolt->InitProjectile(Dir, Speed, Damage, InstigatorController);
}

void ALCBossLich::ExecuteSoulBind(AActor* Target)
{
	// 0) 서버 전용 & 입력 유효성 검사
	if (!HasAuthority() || !Target)
		return;

	// 1) 캐스트 검사
	auto* C = Cast<ABaseCharacter>(Target);
	if (!C)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteSoulBind 시작 → 대상: %s"), *C->GetName());

	// 2) 모든 클라이언트에 FX/SFX & 스턴 재생
	Multicast_PlaySoulBindEffects(C);
}

void ALCBossLich::Multicast_PlaySoulBindEffects_Implementation(ACharacter* Target)
{
	// 0) 대상 및 월드 유효성 검사
	if (!IsValid(Target))
		return;
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 로컬 클라이언트에서만 입력 비활성화
	if (APlayerController* PC = Cast<APlayerController>(Target->GetController()))
	{
		if (PC->IsLocalController())
		{
			Target->DisableInput(PC);

			// 2) 2초 후 입력 복구
			FTimerHandle ReEnableHandle;
			World->GetTimerManager().SetTimer(
				ReEnableHandle,
				FTimerDelegate::CreateLambda([Target, PC]()
					{
						if (IsValid(Target) && PC->IsLocalController())
							Target->EnableInput(PC);
					}),
				2.0f,
				false
			);
		}
	}

	// 3) FX 재생
	if (SoulBindFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			World,
			SoulBindFX,
			Target->GetActorLocation(),
			FRotator::ZeroRotator,
			true
		);
	}

	// 4) SFX 재생 (PlaySoundAtLocation on each client for attenuation)
	if (SoulBindSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,                             // WorldContextObject
			SoulBindSound,                    // 소리 에셋
			Target->GetActorLocation(),       // 재생 위치
			1.f,                              // Volume Multiplier
			1.f,                              // Pitch Multiplier
			0.f,                              // Start Time
			AttackAttenuation               // 어테뉴에이션 에셋
		);
	}
}



void ALCBossLich::ExecuteSoulAbsorb(AActor* Target)
{
	// 0) 서버 전용 및 파라미터 유효성 검사
	if (!HasAuthority() || !Target)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 대미지 값 검사
	const float Damage = SoulAbsorbDamage;
	if (Damage <= 0.f)
		return;

	// 2) 대미지 적용
	AController* InstigatorController = GetController();
	UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteSoulAbsorb 시작 → 대상: %s, Damage=%.1f"),
		*Target->GetName(), Damage);
	UGameplayStatics::ApplyDamage(Target, Damage, InstigatorController, this, nullptr);

	// 3) 모든 클라이언트에 FX/SFX 재생 요청
	Multicast_PlaySoulAbsorbEffects();
}

void ALCBossLich::ExecuteDeathNova()
{
	// 0) 서버 권한 및 World 체크
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 파라미터 유효성 검사
	if (DeathNovaDamage <= 0.f)
		return;
	if (DeathNovaStunDuration < 0.f)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteDeathNova 시작 → Damage=%.1f, StunDuration=%.1f"),
		DeathNovaDamage, DeathNovaStunDuration);

	// 2) 모든 클라이언트에 FX/SFX 재생 요청
	Multicast_PlayDeathNovaEffects();

	// 3) 모든 Pawn 가져와 처리
	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllPawns);

	AController* InstigatorController = GetController();
	for (AActor* Actor : AllPawns)
	{
		if (!Actor || Actor == this)
			continue;

		if (auto* C = Cast<ABaseCharacter>(Actor))
		{
			// (a) 대미지 적용
			UGameplayStatics::ApplyDamage(
				C,
				DeathNovaDamage,
				InstigatorController,
				this,
				nullptr
			);
			UE_LOG(LogTemp, Warning, TEXT("[Lich] DeathNova → %s 입혔습니다"), *C->GetName());

			// (b) 스턴: 이동 비활성화
			if (UCharacterMovementComponent* MoveComp = C->GetCharacterMovement())
			{
				MoveComp->DisableMovement();

				// (c) 일정 시간 후 스턴 해제
				FTimerHandle StunHandle;
				FTimerDelegate RestoreDel = FTimerDelegate::CreateLambda([MoveComp]()
					{
						if (MoveComp)
							MoveComp->SetMovementMode(MOVE_Walking);
					});
				World->GetTimerManager().SetTimer(
					StunHandle,
					RestoreDel,
					DeathNovaStunDuration,
					false
				);
			}
		}
	}
}

void ALCBossLich::Multicast_PlayDeathNovaEffects_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 사운드 (어테뉴에이션 적용)
	if (DeathNovaSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DeathNovaSound,
			GetActorLocation(),
			1.f,                       // Volume
			1.f,                       // Pitch
			0.f,                       // StartTime
			DeathNovaAttenuation       // Attenuation
		);
	}

	if (LightningFX || LightningSound)
	{
		TArray<AActor*> Characters;
		UGameplayStatics::GetAllActorsOfClass(World, ABaseCharacter::StaticClass(), Characters);

		for (AActor* Actor : Characters)
		{
			if (Actor == this) continue;
			auto* Ch = Cast<ABaseCharacter>(Actor);
			if (!Ch) continue;

			// ▲ 아래 라인 추가: 캐릭터 아래 지면을 찾기 위해 라인 트레이스
			FVector TraceStart = Ch->GetActorLocation() + FVector(0, 0, 500.f);
			FVector TraceEnd = Ch->GetActorLocation() - FVector(0, 0, 500.f);
			FHitResult Hit;
			FCollisionQueryParams Params(NAME_None, /*bTraceComplex=*/true, this);
			FVector SpawnLoc = Ch->GetActorLocation(); // fallback

			if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
			{
				SpawnLoc = Hit.Location;
			}

			// ▶ 이제 SpawnLoc 은 바닥 위치

			// 번개 VFX
			if (LightningFX)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					World,
					LightningFX,
					SpawnLoc,
					FRotator::ZeroRotator,
					FVector(1.f)
				);
			}

			// 번개 소리 + 어테뉴에이션
			if (LightningSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					LightningSound,
					SpawnLoc,
					1.f,
					1.f,
					0.f,
					LightningAttenuation
				);
			}
		}
	}
}

void ALCBossLich::Multicast_PlaySoulAbsorbEffects_Implementation()
{
	// 0) World 유효성 검사
	UWorld* World = GetWorld();
	if (!World)
		return;

	const FVector Loc = GetActorLocation();

	// 1) FX 재생 (Auto destroy)
	if (SoulAbsorbFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			World,
			SoulAbsorbFX,
			Loc,
			FRotator::ZeroRotator,
			true  // Auto destroy
		);
	}

	// 2) SFX 재생 (PlaySoundAtLocation → 어테뉴에이션 적용 가능)
	if (SoulAbsorbSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			SoulAbsorbSound,
			Loc,
			1.f,                              // Volume Multiplier
			1.f,                              // Pitch Multiplier
			0.f,                              // Start Time
			AttackAttenuation               // 어테뉴에이션 에셋
		);
	}
}

bool ALCBossLich::RequestAttack(float TargetDistance)
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

	// 4) Death Nova 우선 발동
	if (!bHasUsedDeathNova && Rage >= DeathNovaThreshold)
	{
		bHasUsedDeathNova = true;
		NextAttackRange = FLT_MAX;  // 범위 무관
		NextAttackAction = [this]()
			{
				ExecuteDeathNova();
			};
		return true;
	}

	// 5) 쿨타임 기준으로 후보군만 수집 (거리 검사 제거)
	struct FEntry { float Weight, Range; TFunction<void()> Action; };
	TArray<FEntry> Entries;

	// Arcane Bolt
	{
		float Cd = bIsBerserk ? ArcaneBoltCooldown * BerserkCooldownFactor : ArcaneBoltCooldown;
		if (Now - LastArcaneBoltTime >= Cd && ArcaneBoltClass)
		{
			Entries.Add({
				3.f,
				ArcaneBoltRange,
			[this, Now, Target]()   // ← Target 추가
			{
				UE_LOG(LogTemp, Warning, TEXT("[Lich] 선택된 공격 → ArcaneBolt"));
				LastArcaneBoltTime = Now;
				ExecuteArcaneBolt(Target);
			}
				});
		}
	}

	// Soul Bind
	{
		float Cd = bIsBerserk ? SoulBindCooldown * BerserkCooldownFactor : SoulBindCooldown;
		if (Now - LastSoulBindTime >= Cd)
		{
			Entries.Add({
				2.f,
				SoulBindRange,
			[this, Now, Target]()   // ← Target 추가
			{
				UE_LOG(LogTemp, Warning, TEXT("[Lich] 선택된 공격 → SoulBind"));
				LastSoulBindTime = Now;
				ExecuteSoulBind(Target);
			}
				});
		}
	}

	// Soul Absorb (fallback)
	{
		float Cd = SoulAbsorbCooldown;
		if (Now - LastSoulAbsorbTime >= Cd)
		{
			Entries.Add({
				1.f,
				SoulAbsorbRange,
			[this, Now, Target]()   // ← Target 추가
			{
				UE_LOG(LogTemp, Warning, TEXT("[Lich] 선택된 공격 → SoulAbsorb"));
				LastSoulAbsorbTime = Now;
				ExecuteSoulAbsorb(Target);
			}
				});
		}
	}

	if (Entries.Num() == 0)
		return false;

	// 6) 가중치 랜덤 선택
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
			return true;  // “선택만” 했으니 true
		}
	}

	return false;
}