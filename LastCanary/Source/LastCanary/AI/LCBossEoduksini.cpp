#include "AI/LCBossEoduksini.h"
#include "AI/LCBaseBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"

ALCBossEoduksini::ALCBossEoduksini()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Darkness sphere
	DarknessSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DarknessSphere"));
	check(DarknessSphere);
	DarknessSphere->SetupAttachment(GetRootComponent());
	DarknessSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DarknessSphere->SetSphereRadius(DarknessRadius);
	DarknessSphere->SetGenerateOverlapEvents(false);

	DarknessSphere->OnComponentBeginOverlap.AddDynamic(this, &ALCBossEoduksini::OnDarknessSphereBeginOverlap);
	DarknessSphere->OnComponentEndOverlap.AddDynamic(this, &ALCBossEoduksini::OnDarknessSphereEndOverlap);

}

void ALCBossEoduksini::BeginPlay()
{
	Super::BeginPlay();

	if (DarknessSphere)
	{
		DarknessSphere->SetGenerateOverlapEvents(true);
	}

}

void ALCBossEoduksini::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 1) 서버 권한 & 월드 체크
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	// 2) MovementComponent 널 체크
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[Eoduksini] Missing CharacterMovementComponent"));
		return;
	}

	UpdateRageAndScale(DeltaSeconds);


	if (bIsBerserk && Rage >= DarknessRageThreshold && !bDarknessActive)
	{
		TryTriggerDarkness();
	}

	// (1) 특수 액션: Night Terror
	if (!bHasUsedNightTerror && Rage >= NightTerrorRageThreshold)
	{
		bHasUsedNightTerror = true;
		UE_LOG(LogTemp, Log, TEXT("[Eoduksini] NightTerror 발동"));
		NightTerror();
	}

}

void ALCBossEoduksini::UpdateBlackboardValues()
{
	Super::UpdateBlackboardValues();
}

void ALCBossEoduksini::EnterBerserkState()
{
	Super::EnterBerserkState();
	UE_LOG(LogTemp, Warning, TEXT("[Eoduksini] Enter Berserk State"));

}

void ALCBossEoduksini::StartBerserk()
{
	// 1) 먼저 원본 값 저장
	if (auto* MoveComp = GetCharacterMovement())
	{
		PrevMaxWalkSpeed = MoveComp->MaxWalkSpeed;
		MoveComp->MaxWalkSpeed *= BerserkMovementMultiplier;
	}

	PrevNormalAttackCooldown = NormalAttackCooldown;
	PrevStrongAttackCooldown = StrongAttackCooldown;
	NormalAttackCooldown *= BerserkCooldownMultiplier;
	StrongAttackCooldown *= BerserkCooldownMultiplier;

	Super::StartBerserk();
}

void ALCBossEoduksini::StartBerserk(float Duration)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 이동 속도 배율 조정
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		PrevMaxWalkSpeed = MoveComp->MaxWalkSpeed;
		MoveComp->MaxWalkSpeed *= BerserkMovementMultiplier;
	}

	// 2) 쿨다운 배율 조정
	PrevNormalAttackCooldown = NormalAttackCooldown;
	PrevStrongAttackCooldown = StrongAttackCooldown;
	NormalAttackCooldown *= BerserkCooldownMultiplier;
	StrongAttackCooldown *= BerserkCooldownMultiplier;

	// 3) 부모 클래스 호출 (Duration 기반 berserk 타이머 설정 등)
	Super::StartBerserk(Duration);

}

void ALCBossEoduksini::EndBerserk()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 부모 클래스 종료 처리
	Super::EndBerserk();
	UE_LOG(LogTemp, Warning, TEXT("[Eoduksini] Exit Berserk State"));

	// 2) 이동 속도 복원
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = PrevMaxWalkSpeed;
	}

	// 3) 쿨다운 복원
	NormalAttackCooldown = PrevNormalAttackCooldown;
	StrongAttackCooldown = PrevStrongAttackCooldown;

}

// --- Darkness overlap handlers ---

void ALCBossEoduksini::OnDarknessSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bDarknessActive || !OtherActor)
	{
		return;  // 어둠 상태가 활성화된 뒤에만 처리
	}

	// ① OtherActor를 Pawn으로 캐스트
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	// ② Pawn에서 PlayerController를 얻음
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// ③ 이미 어둠 효과가 적용된 플레이어는 무시
	if (DarkenedPlayers.Contains(PC))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OverlapBegin] 이미 DarkenedPlayers에 있음 → 리턴"));
		return;
	}

	// Fade In 적용
	if (PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.f, DarknessFadeAlpha, FadeDuration, FLinearColor::Black, false, true);
	}

	DarkenedPlayers.Add(PC);
}

void ALCBossEoduksini::OnDarknessSphereEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// 전역 어둠 중이라면 로컬 어둠 해제 무시
	if (bDarknessActive || !OtherActor)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!DarkenedPlayers.Contains(PC)) return;

	if (PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(DarknessFadeAlpha, 0.f, FadeDuration, FLinearColor::Black, false, false);
	}

	DarkenedPlayers.Remove(PC);
}

// --- gaze checking ---

bool ALCBossEoduksini::IsLookedAtByAnyPlayer() const
{
	UWorld* World = GetWorld();
	if (!World)
		return false;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (IsPlayerLooking(It->Get()))
			return true;
	}
	return false;
}

bool ALCBossEoduksini::IsPlayerLooking(APlayerController* PC) const
{
	if (!PC)
		return false;

	FVector ViewLoc;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewLoc, ViewRot);

	// 1) 각도 계산용 도트 연산 안전하게 클램프
	const FVector ToBossDir = (GetActorLocation() - ViewLoc).GetSafeNormal();
	float Dot = FVector::DotProduct(ViewRot.Vector(), ToBossDir);
	Dot = FMath::Clamp(Dot, -1.f, 1.f);

	const float Angle = FMath::RadiansToDegrees(acosf(Dot));

	// 2) 시야 각도 및 시야선 검사
	return Angle <= LookAngleDeg
		&& PC->LineOfSightTo(this);
}

// --- Rage & scale update ---

void ALCBossEoduksini::UpdateRageAndScale(float DeltaSeconds)
{
	// 1) 서버 권한 & 월드 체크
	if (!HasAuthority()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	// 2) 플레이어 시선 집계
	int32 LookCount = CountPlayersLooking();

	// 3) 변화량 계산
	float DeltaRage = ComputeRageDelta(DeltaSeconds, LookCount);

	// 4) Rage 적용 & Berserk 자동 진입
	ApplyRageDelta(DeltaRage);
}

int32 ALCBossEoduksini::CountPlayersLooking() const
{
	UWorld* World = GetWorld();
	if (!World) return 0;

	int32 Total = 0;
	int32 Lookers = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* Pawn = PC->GetPawn();
		if (!Pawn || !Pawn->IsPlayerControlled()) continue;

		++Total;
		if (IsPlayerLooking(PC))
			++Lookers;
	}
	return Lookers;
}

float ALCBossEoduksini::ComputeRageDelta(float DeltaSeconds, int32 LookCount) const
{
	// 기본 증가량
	float Delta = RageGainPerSec * DeltaSeconds;

	// Berserk 상태라면 추가 배수
	if (bIsBerserk)
		Delta *= BerserkRageGainMultiplier;

	// 시선 수만큼 감소
	Delta -= LookCount * RageLossPerSec * DeltaSeconds;
	return Delta;
}

void ALCBossEoduksini::ApplyRageDelta(float DeltaRage)
{
	if (FMath::IsNearlyZero(DeltaRage)) return;

	float NewRage = FMath::Clamp(Rage + DeltaRage, 0.f, MaxRage);
	// (필요하다면 여기서 OnRageChanged 이벤트나 UpdateBlackboardValues() 호출)

	Rage = NewRage;

	UpdateBlackboardValues();

	// 자동 Berserk 진입
	if (Rage >= MaxRage && !bIsBerserk)
	{
		StartBerserk(BerserkDuration);
		UpdateBlackboardValues();
	}
}

// --- Darkness state ---

void ALCBossEoduksini::TryTriggerDarkness()
{
	if (!HasAuthority()) return;

	bDarknessActive = true;
	// 화면 페이드 → 모든 클라가 실행
	Multicast_StartDarkness();

	// 타이머 예약
	GetWorldTimerManager().SetTimer(
		DarknessTimerHandle,
		this, &ALCBossEoduksini::EndDarkness,
		DarknessDuration, false);
}

void ALCBossEoduksini::EndDarkness()
{
	if (!HasAuthority() || !bDarknessActive) return;

	bDarknessActive = false;
	// 화면 페이드 아웃
	Multicast_EndDarkness();

}

void ALCBossEoduksini::OnRep_DarknessActive()
{
	if (bDarknessActive) BP_StartDarknessEffect();
	else                BP_EndDarknessEffect();
}

void ALCBossEoduksini::Multicast_StartDarkness_Implementation()
{
	BP_StartDarknessEffect();
}

void ALCBossEoduksini::Multicast_EndDarkness_Implementation()
{
	BP_EndDarknessEffect();
}

void ALCBossEoduksini::BP_StartDarknessEffect_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[Darkness] 클라이언트: 화면 어둡게 처리 시작"));

	UWorld* World = GetWorld();
	if (!World) return;

	// 1) 이 클라이언트에서 로컬 컨트롤러를 찾아 페이드 처리

	for (auto It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController())
			continue;

		// 각 로컬 플레이어 화면에 Fade In 적용
		PC->PlayerCameraManager->StartCameraFade(
			0.f,                  // 시작 Alpha
			DarknessFadeAlpha,    // 목표 Alpha
			FadeDuration,         // 페이드 시간
			FLinearColor::Black,  // Black으로 페이드
			false,                // bHoldWhenFinished = false
			true                  // bFadeAudio = true (필요 시 사운드도 페이드)
		);
	}

	// 2) 어둠 입장 FX
	if (DarknessEnterFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			DarknessEnterFX,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}

	// 어둠 시작 사운드
	if (DarknessEnterSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DarknessEnterSound,
			GetActorLocation()
		);
	}

}

void ALCBossEoduksini::BP_EndDarknessEffect_Implementation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 1) 화면 페이드 아웃
	for (auto It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController() || !PC->PlayerCameraManager)
			continue;

		APawn* Pawn = PC->GetPawn();
		if (!Pawn) continue;

		float DistToBoss = FVector::Dist(Pawn->GetActorLocation(), GetActorLocation());
		if (DistToBoss < DarknessRadius) continue;

		PC->PlayerCameraManager->StartCameraFade(
			DarknessFadeAlpha, 0.f,
			FadeDuration, FLinearColor::Black,
			false, false
		);
	}

	// 2) 어둠 종료 FX
	if (DarknessExitFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			DarknessExitFX,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}

	// 어둠 종료 사운드
	if (DarknessExitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DarknessExitSound,
			GetActorLocation()
		);
	}
}

// --- Abilities ---

void ALCBossEoduksini::ShadowEcho()
{
	UE_LOG(LogTemp, Log, TEXT("[Eoduksini] ShadowEcho 시작 (딜레이: %.1f초)"), ShadowEchoDelay);

	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
		return;

	// 1) 데미지 예약
	FVector EchoLoc = GetActorLocation() + GetActorForwardVector() * 500.f;
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(
		this, &ALCBossEoduksini::ExecuteShadowEchoDamage, EchoLoc
	);
	World->GetTimerManager().SetTimer(
		ShadowEchoDamageHandle,
		Delegate,
		ShadowEchoDelay,
		false
	);
}


void ALCBossEoduksini::ExecuteShadowEchoDamage(FVector Location)
{
	UWorld* World = GetWorld();
	AController* InstigatorCon = GetController();
	if (!World || !InstigatorCon)
		return;

	UE_LOG(LogTemp, Log, TEXT("[Eoduksini] ShadowEcho 폭발 데미지 수행 (위치: %s)"), *Location.ToCompactString());

	constexpr float Radius = 400.f;

	// 1) 서버에서 데미지 처리
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	bool bHit = World->SweepMultiByChannel(
		Hits,
		Location, Location,
		FQuat::Identity,
		ECC_Pawn,
		Sphere
	);
	if (bHit)
	{
		for (auto& Hit : Hits)
		{
			if (APawn* P = Cast<APawn>(Hit.GetActor()))
			{
				if (P->IsPlayerControlled())
				{
					UGameplayStatics::ApplyDamage(P, EchoDamage, InstigatorCon, this, nullptr);
					if (auto* Ch = Cast<ABaseCharacter>(P))
					{
						if (auto* MoveComp = Ch->GetCharacterMovement())
						{
							MoveComp->MaxWalkSpeed *= 0.5f;
						}
					}
				}
			}
		}
	}

	// 2) 모든 클라이언트에서 FX/Sound 재생
	Multicast_OnShadowEcho(Location);
}

void ALCBossEoduksini::Multicast_OnShadowEcho_Implementation(const FVector& Location)
{
	// 서버에서는 재생하지 않고, 클라이언트에서만
	if (GetNetMode() != NM_Client)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// FX
	if (EchoExplosionFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			EchoExplosionFX,
			Location,
			FRotator::ZeroRotator
		);
	}

	// Sound (3D Attenuation)
	if (EchoExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EchoExplosionSound,
			Location,
			1.f, 1.f, 0.f,
			AttackSoundAttenuation
		);
	}
}

void ALCBossEoduksini::NightmareGrasp()
{
	UE_LOG(LogTemp, Log, TEXT("[Eoduksini] NightmareGrasp 실행"));

	UWorld* World = GetWorld();
	if (!HasAuthority() || !World) return;

	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 800.f;
	FHitResult Hit;
	FCollisionQueryParams Params(NAME_None, false, this);
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
	{

		if (APawn* P = Cast<APawn>(Hit.GetActor()))
		{
			UGameplayStatics::ApplyDamage(P, GraspDamage, GetController(), this, nullptr);
			// stun: disable movement briefly
			if (auto* Ch = Cast<ABaseCharacter>(P))
			{
				Ch->GetCharacterMovement()->DisableMovement();
				FTimerHandle Unused;
				GetWorldTimerManager().SetTimer(Unused, [Ch]() { Ch->GetCharacterMovement()->SetMovementMode(MOVE_Walking); }, 1.5f, false);
			}
		}
	}

	// 2) FX/Sound 는 Multicast 로 클라이언트 재생
	Multicast_OnNightmareGrasp(Hit.ImpactPoint);
}

void ALCBossEoduksini::Multicast_OnNightmareGrasp_Implementation(const FVector& ImpactPoint)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// FX
	if (GraspFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			GraspFX,
			ImpactPoint,
			FRotator::ZeroRotator
		);
	}

	// Sound (3D 감쇠 적용)
	if (GraspSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			GraspSound,
			GetActorLocation(),
			1.f,                     // Volume
			1.f,                     // Pitch
			0.f,                     // StartTime
			AttackSoundAttenuation   // AttenuationSettings
		);
	}
}

void ALCBossEoduksini::NightTerror()
{
	UE_LOG(LogTemp, Log, TEXT("[Eoduksini] NightTerror 실행"));

	UWorld* World = GetWorld();
	AController* InstigatorCon = GetController();
	if (!HasAuthority() || !World || !InstigatorCon)
	{
		return;
	}

	// 1) 서버에서 데미지 처리
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* Pawn = PC->GetPawn();
		if (!Pawn) continue;

		UGameplayStatics::ApplyDamage(
			Pawn,
			TerrorDamage,
			InstigatorCon,
			this,
			nullptr
		);
	}

	// 2) 이펙트 재생
	Multicast_NightTerrorEffects();
}

void ALCBossEoduksini::Multicast_NightTerrorEffects_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// FX
	if (TerrorFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			TerrorFX,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}

	// SFX
	if (TerrorSound)
	{
		UGameplayStatics::PlaySound2D(this, TerrorSound);
	}

	// PostProcess
	if (!TerrorPostProcessMaterial)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue;

		APawn* Target = PC->GetPawn();
		if (!Target) continue;

		UCameraComponent* Cam = Target->FindComponentByClass<UCameraComponent>();
		if (!Cam) continue;

		// Blend in
		Cam->PostProcessSettings.AddBlendable(
			TerrorPostProcessMaterial,
			TerrorPostProcessWeight
		);

		// 일정 시간 뒤 Blend 제거
		FTimerHandle RemoveHandle;
		World->GetTimerManager().SetTimer(
			RemoveHandle,
			FTimerDelegate::CreateWeakLambda(this, [Cam, this]()
				{
					if (Cam)
					{
						Cam->PostProcessSettings.RemoveBlendable(TerrorPostProcessMaterial);
					}
				}),
			TerrorPostProcessDuration,
			false
		);
	}
}

// --- basic attacks ---

void ALCBossEoduksini::ShadowSwipe()
{
	UE_LOG(LogTemp, Log, TEXT("[Eoduksini] ShadowSwipe 실행: 대미지 %.1f"), SwipeDamage);

	// 1) 데미지 처리
	DealDamageInRange(SwipeDamage);

	// 2) 모든 클라이언트에서 FX/Sound 재생
	Multicast_OnShadowSwipe();

}

void ALCBossEoduksini::Multicast_OnShadowSwipe_Implementation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 로컬 변수 이름을 MeshComp로 변경
	USkeletalMeshComponent* MeshComp = GetMesh();

	// FX (Niagara)
	if (SwipeFX && MeshComp)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			SwipeFX,
			MeshComp,             // 변경된 이름 사용
			TEXT("spine_01"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// Sound (3D Attenuation)
	if (SwipeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			SwipeSound,
			GetActorLocation(),
			1.f,                // Volume
			1.f,                // Pitch
			0.f,                // StartTime
			AttackSoundAttenuation
		);
	}
}

void ALCBossEoduksini::VoidGrasp()
{
	UE_LOG(LogTemp, Log, TEXT("[Eoduksini] VoidGrasp 실행: 대미지 %.1f"), GraspDamage);

	UWorld* World = GetWorld();
	AController* InstigatorCon = GetController();
	if (!World || !InstigatorCon) return;

	FVector BossLoc = GetActorLocation();
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(VoidGraspRange);

	// 반경 내 Pawn 검색
	if (World->SweepMultiByChannel(Hits, BossLoc, BossLoc, FQuat::Identity, ECC_Pawn, Sphere))
	{
		for (auto& Hit : Hits)
		{
			if (APawn* P = Cast<APawn>(Hit.GetActor()))
			{
				// 당기는 방향 계산
				FVector PullDir = (BossLoc - P->GetActorLocation()).GetSafeNormal();

				// 캐릭터라면 LaunchCharacter 로 물리 이동
				if (auto* Ch = Cast<ABaseCharacter>(P))
				{
					const float PullStrength = 1500.f; // 필요에 따라 조정
					// XY/Z 모두 덮어쓰도록 true,true
					Ch->LaunchCharacter(PullDir * PullStrength, true, true);
				}
				// 만약 물리 시뮬레이션 컴포넌트라면 AddImpulse 사용
				else if (UPrimitiveComponent* Prim = Hit.GetComponent())
				{
					if (Prim->IsSimulatingPhysics())
					{
						const float ImpulseStrength = 800.f;
						Prim->AddImpulse(PullDir * ImpulseStrength, NAME_None, true);
					}
				}

				// 대미지 적용
				UGameplayStatics::ApplyDamage(
					P,
					GraspDamage,
					GetController(),
					this,
					nullptr
				);
			}
		}
	}

	// 2) 모든 클라이언트에서 FX/Sound 재생
	Multicast_OnVoidGrasp();
}

void ALCBossEoduksini::Multicast_OnVoidGrasp_Implementation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// FX (Niagara)
	if (VoidGraspFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			VoidGraspFX,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}

	// Sound (3D Attenuation)
	if (VoidGraspSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			VoidGraspSound,
			GetActorLocation(),
			1.f,
			1.f,
			0.f,
			AttackSoundAttenuation
		);
	}
}

// --- override base attack to include rage logic ---

bool ALCBossEoduksini::RequestAttack(float TargetDistance)
{
	if (!HasAuthority()) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	const float Now = GetWorld()->GetTimeSeconds();
	struct FEntry { float Weight; TFunction<void()> Action; };
	TArray<FEntry> Entries;

	// 1) ShadowEcho
	if (TargetDistance <= ShadowEchoRange
		&& Now - LastShadowEchoTime >= ShadowEchoInterval
		&& ShadowEchoWeight > 0.f)
	{
		Entries.Add({ ShadowEchoWeight, [this, Now]()
		{
			LastShadowEchoTime = Now;
			ShadowEcho();
		} });
	}

	// 2) NightmareGrasp
	if (TargetDistance <= NightmareGraspRange && Now - LastNightmareGraspTime >= NightmareGraspInterval && NightmareGraspWeight > 0.f)
	{
		Entries.Add({ NightmareGraspWeight, [this, Now]()
		{
			LastNightmareGraspTime = Now;
			UE_LOG(LogTemp, Log, TEXT("[Eodu] NightmareGrasp 실행"));
			NightmareGrasp();
		} });
	}

	// 3) 근접계열 (ShadowSwipe)
	if (TargetDistance <= ShadowSwipeRange && Now - LastNormalTime >= NormalAttackCooldown && ShadowSwipeWeight > 0.f)
	{
		Entries.Add({ ShadowSwipeWeight, [this, Now]()
		{
			LastNormalTime = Now;
			UE_LOG(LogTemp, Log, TEXT("[Eodu] ShadowSwipe 실행"));
			ShadowSwipe();
		} });
	}

	// 4) 견인계열 (VoidGrasp)
	if (TargetDistance <= VoidGraspRange && Now - LastStrongTime >= StrongAttackCooldown && VoidGraspWeight > 0.f)
	{
		Entries.Add({ VoidGraspWeight, [this, Now]()
		{
			LastStrongTime = Now;
			UE_LOG(LogTemp, Log, TEXT("[Eodu] VoidGrasp 실행"));
			VoidGrasp();
		} });
	}

	// 가중치 랜덤 선택
	float TotalW = 0.f;
	for (auto& E : Entries) TotalW += E.Weight;
	if (TotalW <= 0.f) return false;

	float Pick = FMath::FRandRange(0.f, TotalW), Acc = 0.f;
	for (auto& E : Entries)
	{
		Acc += E.Weight;
		if (Pick <= Acc)
		{
			E.Action();
			return true;
		}
	}

	return false;
}

void ALCBossEoduksini::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCBossEoduksini, bDarknessActive);
}