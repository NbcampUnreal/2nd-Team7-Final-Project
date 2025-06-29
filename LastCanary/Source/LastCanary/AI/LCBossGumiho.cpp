#include "AI/LCBossGumiho.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"
#include "Math/UnrealMathUtility.h"
#include "AI/LCBaseBossAIController.h"
#include "Character/BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/Summon/Illusion.h"
#include "Camera/CameraComponent.h"

ALCBossGumiho::ALCBossGumiho()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ALCBossGumiho::BeginPlay()
{
	Super::BeginPlay();

	// Illusion Spawn
	GetWorldTimerManager().SetTimer(IllusionTimerHandle, this, &ALCBossGumiho::SpawnIllusions, IllusionInterval, true);

	// Charm Gaze
	GetWorldTimerManager().SetTimer(CharmTimerHandle, this, &ALCBossGumiho::ExecuteCharmGaze, CharmInterval, true);
}

void ALCBossGumiho::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 0) 서버 권한 및 DeltaTime 유효성 검사
	if (!HasAuthority() || DeltaTime <= 0.f)
		return;

	// 1) Rage 갱신 (부모 로직 포함)
	UpdateRage(DeltaTime);
}

void ALCBossGumiho::UpdateBlackboardValues()
{
	Super::UpdateBlackboardValues();
}

void ALCBossGumiho::UpdateRage(float DeltaSeconds)
{
	if (!HasAuthority()) return;

	Super::UpdateRage(DeltaSeconds);

	// (1) 살아있는 환영 수에 비례해 Rage 증가
	int32 IllCount = IllusionActors.Num();

	if (IllCount > 0)
	{
		const float Delta = IllusionRagePerSecond * IllCount * DeltaSeconds;
		AddRage(Delta);
	}

	// (2) 매혹된 플레이어 수에 비례해 Rage 감소 (SweepMultiByChannel 사용)
	TArray<FHitResult> HitResults;
	FVector Origin = GetActorLocation();
	FCollisionShape Sphere = FCollisionShape::MakeSphere(CharmRadius);

	// 시작과 끝을 동일하게 주면, 단순 범위 검사처럼 동작
	if (GetWorld()->SweepMultiByChannel(
		HitResults,
		Origin,
		Origin,
		FQuat::Identity,
		ECC_Pawn,
		Sphere))
	{
		int32 CharmCount = 0;
		for (const auto& Hit : HitResults)
		{
			if (APawn* P = Cast<APawn>(Hit.GetActor()))
			{
				if (P->Tags.Contains(FName("Charmed")))
				{
					CharmCount++;
				}
			}
		}
		if (CharmCount > 0)
		{
			AddRage(-CharmRagePerSecond * CharmCount * DeltaSeconds);
		}
	}

	// Nine-Tail Burst
	if (HasAuthority() && !bHasUsedNineTail && Rage >= NineTailBurstRageThreshold)
	{
		bHasUsedNineTail = true;
		ExecuteNineTailBurst();
	}
}

void ALCBossGumiho::AddRage(float Amount)
{
	if (!HasAuthority())
		return;

	// Rage 갱신
	Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);

	// 2) 서버 권한이 있을 때만 Blackboard 업데이트

	UpdateBlackboardValues();


	// MaxRage 도달 시 광폭화
	if (Rage >= MaxRage && !bIsBerserk)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Rage가 최대치에 도달하여 Berserk 모드 진입"));
		StartBerserk(BerserkDuration);
		UpdateBlackboardValues();
	}
}

void ALCBossGumiho::EnterBerserkState()
{
	Super::EnterBerserkState();
	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Enter Berserk State"));


	// (2) 이동 속도·공격력 버프 적용
	GetCharacterMovement()->MaxWalkSpeed *= BerserkSpeedMultiplier;
	NormalAttackDamage *= BerserkDamageMultiplier;
}

void ALCBossGumiho::StartBerserk()
{
	Super::StartBerserk();

}

void ALCBossGumiho::StartBerserk(float Duration)
{
	Super::StartBerserk(Duration);

}

void ALCBossGumiho::EndBerserk()
{
	Super::EndBerserk();
	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] End Berserk State"));


	// (6) 버프 수치 원상복구
	GetCharacterMovement()->MaxWalkSpeed /= BerserkSpeedMultiplier;
	NormalAttackDamage /= BerserkDamageMultiplier;
}

void ALCBossGumiho::SpawnIllusions()
{
	// 0) 서버 권한 및 World 유효성 검사
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
		return;

	// 1) IllusionClass 및 NumIllusions 유효성 검사
	if (!IllusionClass || NumIllusions <= 0)
		return;

	// 2) 이미 죽은(Invalid) 분신 정리
	IllusionActors.RemoveAll([](AIllusion* I) {
		return !IsValid(I);
		});

	// 3) 현재 소환된 분신 수 확인
	int32 CurrentCount = IllusionActors.Num();
	if (CurrentCount >= NumIllusions)
		return;

	int32 ToSpawn = NumIllusions - CurrentCount;

	// 4) 스폰 파라미터 설정
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = Cast<APawn>(GetController() ? GetController()->GetPawn() : nullptr);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 5) 분신 소환 루프
	for (int32 i = 0; i < ToSpawn; ++i)
	{
		// (a) 랜덤 위치: 보스 주변 박스 내
		FVector Origin = GetActorLocation();
		FVector Min(-600.f, -600.f, 0.f), Max(600.f, 600.f, 0.f);
		FVector Loc = Origin + FMath::RandPointInBox(FBox(Min, Max));

		// (b) 실제 스폰
		AIllusion* Ill = World->SpawnActor<AIllusion>(
			IllusionClass,
			Loc,
			GetActorRotation(),
			Params
		);
		if (!IsValid(Ill))
			continue;

		// (c) 소멸 콜백 및 배열 추가
		Ill->OnDestroyed.AddDynamic(this, &ALCBossGumiho::OnIllusionDestroyed);
		IllusionActors.Add(Ill);

		// (d) 보스 참조 설정
		Ill->SetBossOwner(this);
	}

	UE_LOG(LogTemp, Log, TEXT("[Gumiho] SpawnIllusions: %d spawned, now %d/%d"),
		ToSpawn, IllusionActors.Num(), NumIllusions);
}

void ALCBossGumiho::OnIllusionDestroyed(AActor* DestroyedActor)
{
	// 0) 서버 권한 및 파라미터 유효성 검사
	if (!HasAuthority() || !DestroyedActor)
		return;

	// 1) 배열에서 이미 파괴된 (Invalid) 분신 정리
	IllusionActors.RemoveAll([](AIllusion* I) {
		return !IsValid(I);
		});

	// 2) DestroyedActor가 분신인지 캐스트 검사
	AIllusion* Ill = Cast<AIllusion>(DestroyedActor);
	if (!Ill)
		return;

	// 3) 실제 배열에 있던 분신인지 확인하며 제거
	if (IllusionActors.Remove(Ill) > 0)
	{
		// 4) 패널티 값이 유효할 때만 Rage 감소
		if (IllusionDeathPenalty > 0.f)
		{
			AddRage(-IllusionDeathPenalty);
		}

		// 5) 바인딩 해제
		Ill->OnDestroyed.RemoveDynamic(this, &ALCBossGumiho::OnIllusionDestroyed);
	}
}

void ALCBossGumiho::ExecuteTailStrike()
{
	// 0) 오직 서버에서만 실행
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 파라미터 유효성 검사
	if (TailStrikeRadius <= 0.f || TailStrikeDamage <= 0.f)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Tail Strike executed: Radius=%.1f, Damage=%.1f"),
		TailStrikeRadius, TailStrikeDamage);

	// 2) 클라이언트 FX/SFX 재생
	Multicast_PlayTailStrikeEffects();

	// 3) 범위 내 Pawn에 데미지 적용
	FVector Origin = GetActorLocation();
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TailStrikeRadius);

	if (World->SweepMultiByChannel(
		Hits,
		Origin, Origin,
		FQuat::Identity,
		ECC_Pawn,
		Sphere))
	{
		AController* InstigatorCtrl = GetController();
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActor == this)
				continue;

			if (ABaseCharacter* C = Cast<ABaseCharacter>(HitActor))
			{
				UGameplayStatics::ApplyDamage(
					C,
					TailStrikeDamage,
					InstigatorCtrl,
					this,
					nullptr
				);
			}
		}
	}
}

void ALCBossGumiho::Multicast_PlayTailStrikeEffects_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	FVector Loc = GetActorLocation();

	// FX
	if (TailStrikeFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			TailStrikeFX,
			Loc,
			FRotator::ZeroRotator,
			FVector(1.f),
			true
		);
	}

	// SFX with attenuation
	if (TailStrikeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			TailStrikeSound,
			Loc,
			1.f,                 // VolumeMultiplier
			1.f,                 // PitchMultiplier
			0.f,                 // StartTime
			AttackAttenuation
		);
	}
}

void ALCBossGumiho::ExecuteFoxfireVolley()
{
	// 0) 서버 전용 및 World 유효성 검사
	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (!World || !FoxfireClass || FoxfireCount <= 0)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Foxfire Volley executed: Count=%d"), FoxfireCount);

	FVector Origin = GetActorLocation();

	// 1) 클라이언트에 FX/SFX 재생 요청
	Multicast_PlayFoxfireVolleyEffects(Origin);

	// 2) Foxfire 스폰
	const float FoxfireRadius = 600.f;
	const float MinZ = 50.f, MaxZ = 150.f;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < FoxfireCount; ++i)
	{
		float Angle = FMath::RandRange(0.f, 2 * PI);
		FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * FoxfireRadius;
		Offset.Z = FMath::RandRange(MinZ, MaxZ);

		FVector SpawnLoc = Origin + Offset;
		FRotator SpawnRot = Offset.Rotation();

		AActor* Spawned = World->SpawnActor<AActor>(FoxfireClass, SpawnLoc, SpawnRot, Params);
		if (!Spawned)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Failed to spawn Foxfire #%d"), i);
		}
	}
}

void ALCBossGumiho::Multicast_PlayFoxfireVolleyEffects_Implementation(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 사운드 (어테뉴에이션 적용)
	if (FoxfireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FoxfireSound,
			Origin,
			1.f,
			1.f,
			0.f,
			AttackAttenuation
		);
	}

}



void ALCBossGumiho::PerformIllusionSwap()
{
	// 0) 서버 권한 및 World 유효성 검사
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World || IllusionActors.Num() == 0)
		return;

	// (1) 환영 중 하나를 랜덤 선택
	int32 IllIdx = FMath::RandRange(0, IllusionActors.Num() - 1);
	AActor* Ill = IllusionActors.IsValidIndex(IllIdx) ? IllusionActors[IllIdx] : nullptr;
	if (!IsValid(Ill))
		return;

	// (2) 반경 내 플레이어 수집
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(IllusionSwapRadius);
	if (!World->SweepMultiByChannel(
		Hits,
		GetActorLocation(), GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere))
		return;

	// (3) 플레이어 Pawn 필터링
	TArray<APawn*> ValidPlayers;
	for (auto& Hit : Hits)
	{
		APawn* P = Cast<APawn>(Hit.GetActor());
		if (P && P->IsPlayerControlled())
			ValidPlayers.Add(P);
	}
	if (ValidPlayers.Num() == 0)
		return;

	// (4) 플레이어 중 하나 랜덤 선택
	int32 PlyIdx = FMath::RandRange(0, ValidPlayers.Num() - 1);
	APawn* TargetPlayer = ValidPlayers.IsValidIndex(PlyIdx) ? ValidPlayers[PlyIdx] : nullptr;
	if (!IsValid(TargetPlayer))
		return;

	// (5) 위치 스왑
	const FVector IllLoc = Ill->GetActorLocation();
	const FVector PlayerLoc = TargetPlayer->GetActorLocation();
	Ill->SetActorLocation(PlayerLoc);
	TargetPlayer->SetActorLocation(IllLoc);

	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Swapped Illusion[%s] with Player[%s]"),
		*Ill->GetName(), *TargetPlayer->GetName());

	// (6) 클라이언트 FX/SFX 재생
	Multicast_PlayIllusionSwapEffects(IllLoc);
}

void ALCBossGumiho::Multicast_PlayIllusionSwapEffects_Implementation(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 디버그: 스왑 지점 시각화 (선택사항)
	DrawDebugSphere(World, Origin, 100.f, 16, FColor::Purple, false, 1.0f);

	// 사운드 재생 (어테뉴에이션 적용)
	if (IllusionSpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			IllusionSpawnSound,
			Origin,
			1.f,                        // Volume
			1.f,                        // Pitch
			0.f,                        // StartTime
			AttackAttenuation     // Attenuation Asset
		);
	}
}


void ALCBossGumiho::ExecuteCharmGaze()
{
	// 0) 서버 권한 및 World 유효성 검사
	if (!HasAuthority())
		return;
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 1) 파라미터 유효성 검사
	if (CharmRadius <= 0.f)
		return;
	if (CharmInterval <= 0.f)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Charm Gaze executed: Radius=%.1f, Interval=%.1f"),
		CharmRadius, CharmInterval);

	// 2) 디버그 스피어 (World 유효성 이미 확인)
	DrawDebugSphere(
		World,
		GetActorLocation(),
		CharmRadius,
		16,
		FColor::Red,
		false,
		2.0f,
		0,
		5.0f
	);

	// 3) SweepMultiByChannel으로 대상 수집
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(CharmRadius);
	if (!World->SweepMultiByChannel(
		Hits,
		GetActorLocation(), GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere))
	{
		return;
	}

	// 4) 플레이어 캐릭터에 태그 적용 및 타이머 등록
	const FName CharmTag(TEXT("Charmed"));
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == this)
			continue;

		ABaseCharacter* P = Cast<ABaseCharacter>(HitActor);
		if (!IsValid(P))
			continue;

		if (!P->Tags.Contains(CharmTag))
		{
			// (a) 태그 추가
			P->Tags.Add(CharmTag);
			UE_LOG(LogTemp, Log, TEXT("[Gumiho] %s is Charmed"), *P->GetName());

			// (b) 일정 시간 후 태그 제거
			FTimerHandle TimerHandle;
			FTimerDelegate RemoveDel = FTimerDelegate::CreateLambda([WeakP = TWeakObjectPtr<ABaseCharacter>(P), CharmTag]()
				{
					if (ABaseCharacter* CP = WeakP.Get())
					{
						CP->Tags.Remove(CharmTag);
						UE_LOG(LogTemp, Log, TEXT("[Gumiho] %s is Uncharmed"), *CP->GetName());
					}
				});
			World->GetTimerManager().SetTimer(
				TimerHandle,
				RemoveDel,
				CharmInterval,
				false
			);
		}
	}
}

void ALCBossGumiho::ExecuteNineTailBurst()
{
	// 0) 서버 권한 및 Rage 조건 검사
	if (!HasAuthority() || bHasUsedNineTail || Rage >= NineTailBurstRageThreshold)
		return;

	UWorld* World = GetWorld();
	if (!World || NineTailBurstDamage <= 0.f)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Nine-Tail Burst activated"));

	// 1) 전 범위 대미지: 모든 ABaseCharacter에 적용
	TArray<AActor*> AllChars;
	UGameplayStatics::GetAllActorsOfClass(World, ABaseCharacter::StaticClass(), AllChars);

	AController* InstigatorCtrl = GetController();
	for (AActor* Actor : AllChars)
	{
		if (!Actor || Actor == this)
			continue;

		if (ABaseCharacter* C = Cast<ABaseCharacter>(Actor))
		{
			UGameplayStatics::ApplyDamage(
				C,
				NineTailBurstDamage,
				InstigatorCtrl,
				this,
				nullptr
			);
			UE_LOG(LogTemp, Log, TEXT("[Gumiho] Nine-Tail Burst → %s hit for %.1f"),
				*C->GetName(), NineTailBurstDamage);
		}
	}

	bHasUsedNineTail = true;
	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Nine-Tail Burst executed"));

	// 2) 포스트프로세스 재생 요청
	Multicast_PlayNineTailBurstEffects();
}

void ALCBossGumiho::Multicast_PlayNineTailBurstEffects_Implementation()
{
	UWorld* World = GetWorld();
	if (!World || !NineTailBurstPPMaterial)
		return;

	// 적용할 시간
	const float Duration = FMath::Max(NineTailBurstPPDuration, 0.1f);

	// 모든 로컬 플레이어 컨트롤러 순회
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (!PC->IsLocalController())
				continue;

			// 카메라 컴포넌트 획득
			if (APawn* Pawn = PC->GetPawn())
			{
				if (UCameraComponent* Cam = Pawn->FindComponentByClass<UCameraComponent>())
				{
					// 1) 블렌더 추가
					FPostProcessSettings& Settings = Cam->PostProcessSettings;
					Settings.AddBlendable(NineTailBurstPPMaterial, NineTailBurstPPWeight);

					// 2) 일정 시간 후 제거
					FTimerHandle TimerHandle;
					PC->GetWorldTimerManager().SetTimer(
						TimerHandle,
						FTimerDelegate::CreateLambda([Cam, this]()
							{
								if (Cam && NineTailBurstPPMaterial)
								{
									Cam->PostProcessSettings.RemoveBlendable(NineTailBurstPPMaterial);
								}
							}),
						Duration,
						false
					);

					// 3) Sound 재생
					if (NineTailBurstSound)
					{
						UGameplayStatics::PlaySoundAtLocation(
							this,
							NineTailBurstSound,
							Cam->GetComponentLocation()
						);
					}
				}
			}
		}
	}
}


void ALCBossGumiho::ExecuteSpiritSpike(AActor* Target)
{
	// 0) 서버 전용 및 파라미터 유효성 검사
	if (!HasAuthority() || !Target)
		return;

	UWorld* World = GetWorld();
	if (!World || SpiritSpikeRadius <= 0.f || SpiritSpikeDamage <= 0.f)
		return;

	FVector Loc = Target->GetActorLocation();

	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] ExecuteSpiritSpike at %s"), *Loc.ToString());

	// 1) 클라이언트 VFX/SFX 재생
	Multicast_PlaySpiritSpikeEffects(Loc);

	// 2) 범위 대미지 처리
	TArray<FHitResult> Hits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SpiritSpikeRadius);

	if (World->SweepMultiByChannel(
		Hits, Loc, Loc, FQuat::Identity,
		ECC_Pawn, Sphere))
	{
		AController* InstigatorCtrl = GetController();
		for (const FHitResult& H : Hits)
		{
			AActor* HitActor = H.GetActor();
			if (!HitActor || HitActor == this)
				continue;

			if (auto* C = Cast<ABaseCharacter>(HitActor))
			{
				UGameplayStatics::ApplyDamage(
					C,
					SpiritSpikeDamage,
					InstigatorCtrl,
					this,
					nullptr
				);
			}
		}
	}
}

void ALCBossGumiho::Multicast_PlaySpiritSpikeEffects_Implementation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// VFX
	if (SpiritSpikeFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			SpiritSpikeFX,
			Location,
			FRotator::ZeroRotator,
			FVector(1.f),  // scale
			true           // auto destroy
		);
	}

	// SFX
	if (SpiritSpikeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			SpiritSpikeSound,
			Location,
			1.f,                       // volume
			1.f,                       // pitch
			0.f,                       // start time
			AttackAttenuation     // attenuation asset
		);
	}
}

void ALCBossGumiho::OnRep_DivineGrace()
{
	if (bIsDivineGrace)
	{
		GetCharacterMovement()->MaxWalkSpeed *= 1.5f;
		UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Divine Grace activated"));
	}
}

void ALCBossGumiho::Multicast_StartDivineGrace_Implementation()
{
	OnRep_DivineGrace();
	// 추가 이펙트/사운드 재생 가능
}

bool ALCBossGumiho::RequestAttack(float TargetDistance)
{
	// 0) 서버 권한 및 World 유효성 검사
	if (!HasAuthority())
		return false;

	UWorld* World = GetWorld();
	if (!World)
		return false;

	// 1) 너무 멀면 공격하지 않음
	if (TargetDistance > 600.f)
		return false;

	// 2) Controller 및 Blackboard 검사
	AController* Ctrl = GetController();
	AAIController* AICon = Ctrl ? Cast<AAIController>(Ctrl) : nullptr;
	UBlackboardComponent* BB = AICon ? AICon->GetBlackboardComponent() : nullptr;

	// 3) Target 획득
	AActor* Target = nullptr;
	if (BB)
	{
		UObject* Obj = BB->GetValueAsObject(TEXT("TargetActor"));
		Target = Obj ? Cast<AActor>(Obj) : nullptr;
	}
	const bool bHasTarget = IsValid(Target);

	const float Now = World->GetTimeSeconds();

	struct FEntry { float Weight; TFunction<void()> Action; };
	TArray<FEntry> Entries;

	// Foxfire Volley
	if (FoxfireInterval > 0.f && Now - LastFoxfireTime >= FoxfireInterval)
	{
		Entries.Add({ 3.f, [this, Now]() {
			LastFoxfireTime = Now;
			ExecuteFoxfireVolley();
		} });
	}

	// Tail Strike
	if (TailStrikeCooldown > 0.f && bHasTarget && TargetDistance <= TailStrikeRadius
		&& Now - LastTailStrikeTime >= TailStrikeCooldown)
	{
		Entries.Add({ 2.f, [this, Now]() {
			LastTailStrikeTime = Now;
			ExecuteTailStrike();
		} });
	}

	// Illusion Swap
	if (IllusionSwapInterval > 0.f && Now - LastIllusionSwapTime >= IllusionSwapInterval)
	{
		Entries.Add({ 1.f, [this, Now]() {
			LastIllusionSwapTime = Now;
			PerformIllusionSwap();
		} });
	}

	// Spirit Spike
	if (SpiritSpikeCooldown > 0.f && bHasTarget && Now - LastSpiritSpikeTime >= SpiritSpikeCooldown)
	{
		Entries.Add({ 2.f, [this, Now, Target]() {
			LastSpiritSpikeTime = Now;
			UE_LOG(LogTemp, Warning, TEXT("[Gumiho] SpiritSpike 실행 → 대상: %s"), *Target->GetName());
			ExecuteSpiritSpike(Target);
		} });
	}

	// 4) 가중치 랜덤 선택
	float TotalWeight = 0.f;
	for (const auto& E : Entries)
	{
		TotalWeight += E.Weight;
	}
	if (TotalWeight <= KINDA_SMALL_NUMBER)
		return false;

	float Pick = FMath::FRandRange(0.f, TotalWeight);
	float Accum = 0.f;
	for (const auto& E : Entries)
	{
		Accum += E.Weight;
		if (Pick <= Accum)
		{
			E.Action();
			return true;
		}
	}

	return false;
}



void ALCBossGumiho::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCBossGumiho, bIsDivineGrace);
}
