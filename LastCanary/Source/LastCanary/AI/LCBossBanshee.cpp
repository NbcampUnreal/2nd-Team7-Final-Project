#include "AI/LCBossBanshee.h"
#include "AI/LCBaseBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/BaseCharacter.h"   
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "AI/Summon/CloneMinion.h"

ALCBossBanshee::ALCBossBanshee()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ALCBossBanshee::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(
		PingTimerHandle,
		this,
		&ALCBossBanshee::EcholocationPing,
		PingInterval,
		true
	);
}

void ALCBossBanshee::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 월드 유효성 검사
	UWorld* World = GetWorld();
	if (!World) return;

	// 서버 권한이 있을 때만 Rage를 감소
	if (HasAuthority())
	{
		DecayRage(DeltaTime);
	}

	// (1) 특수 스킬 - Desperate Wail (한 번만)
	if (!bHasUsedDesperateWail && Rage >= DesperateWailRageThreshold)
	{
		bHasUsedDesperateWail = true;
		UE_LOG(LogTemp, Log, TEXT("[Banshee] Desperate Wail 발동"));
		DesperateWail();
	}
}

void ALCBossBanshee::UpdateBlackboardValues()
{
	Super::UpdateBlackboardValues();
}


void ALCBossBanshee::EcholocationPing()
{
	// 1) 월드 체크
	UWorld* World = GetWorld();
	if (!World) return;

	// 2) 사운드는 멀티캐스트로
	Multicast_PlayEcholocationSound(GetActorLocation());

	// 3) 반경 계산
	const float Radius = bIsBerserk ? PingRadius * 1.2f : PingRadius;
	const FVector Origin = GetActorLocation();

	// 4) 디버그 스피어
	DrawDebugSphere(World, Origin, Radius, 32, FColor::Cyan, false, RevealDuration);

	// 5) 충돌 검사 준비
	TArray<FHitResult> HitResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bRevealedAnyone = false;
	bool bFoundPingTarget = false;

	// 6) SweepMultiByObjectType
	if (World->SweepMultiByObjectType(
		HitResults,
		Origin, Origin,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		Sphere,
		Params))
	{
		for (const FHitResult& Hit : HitResults)
		{
			auto* HitChar = Cast<ABaseCharacter>(Hit.GetActor());
			if (!HitChar) continue;

			// 이름을 Mesh → SkelMesh 로 변경
			if (USkeletalMeshComponent* SkelMesh = HitChar->GetMesh())
			{
				SkelMesh->SetRenderCustomDepth(true);
				SkelMesh->SetCustomDepthStencilValue(252);
			}

			HandleRehide(HitChar);
			bRevealedAnyone = true;

			if (!bFoundPingTarget)
			{
				LastPingedLocation = HitChar->GetActorLocation();
				bFoundPingTarget = true;
			}
		}
	}

	// 7) Rage 증감
	AddRage(bRevealedAnyone ? PingRageGain : -PingRageLoss);
}

void ALCBossBanshee::Multicast_PlayEcholocationSound_Implementation(const FVector& Location)
{
	if (!EcholocationSound) return;

	// 감쇠 세팅이 있으면 넣고, 없으면 nullptr(전역)로 동작
	UGameplayStatics::PlaySoundAtLocation(
		this,
		EcholocationSound,
		Location,
		/*Volume*/1.f,
		/*Pitch*/1.f,
		/*StartTime*/0.f,
		EcholocationAttenuation  // ← 여기에 감쇠 에셋 지정
	);
}

void ALCBossBanshee::EnterBerserkState()
{
	Super::EnterBerserkState();
	UE_LOG(LogTemp, Warning, TEXT("[Banshee] 영구 Berserk 진입"));
}

void ALCBossBanshee::StartBerserk()
{
	Super::StartBerserk();

	// 1) 원래 값 저장
	PrevPingRadius = PingRadius;
	PrevShriekCooldown = ShriekCooldown;
	PrevWailRange = WailRange;

	// 2) 멀티플라이어 적용
	PingRadius *= BerserkPingRadiusMultiplier;
	ShriekCooldown *= BerserkShriekCooldownMultiplier;
	WailRange *= BerserkWailRangeMultiplier;

	// 3) 분신 소환
	SpawnBansheeClones();
}

void ALCBossBanshee::StartBerserk(float Duration)
{
	Super::StartBerserk(Duration);

	// 동일하게 버서크 스탯 버프
	PrevPingRadius = PingRadius;
	PrevShriekCooldown = ShriekCooldown;
	PrevWailRange = WailRange;

	PingRadius *= BerserkPingRadiusMultiplier;
	ShriekCooldown *= BerserkShriekCooldownMultiplier;
	WailRange *= BerserkWailRangeMultiplier;

	// 3) 분신 소환
	SpawnBansheeClones();
}

void ALCBossBanshee::EndBerserk()
{
	Super::EndBerserk();

	// 버퍼 해제: 원래 값으로 복구
	PingRadius = PrevPingRadius;
	ShriekCooldown = PrevShriekCooldown;
	WailRange = PrevWailRange;

	UE_LOG(LogTemp, Warning, TEXT("[Banshee] Berserk End → PingRadius: %.1f, ShriekCD: %.1f, WailRange: %.1f"),
		PingRadius, ShriekCooldown, WailRange);

}

void ALCBossBanshee::HandleRehide(ACharacter* Char)
{
	UWorld* World = GetWorld();
	if (!World || !Char) return;

	FTimerHandle TempHandle;
	World->GetTimerManager().SetTimer(TempHandle, [Char]()
		{
			if (IsValid(Char))
			{
				if (USkeletalMeshComponent* Mesh = Char->GetMesh())
				{
					Mesh->SetRenderCustomDepth(false);
				}
			}
		}, RevealDuration, false);
}

void ALCBossBanshee::OnHeardNoise(const FVector& NoiseLocation)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector Dir = NoiseLocation - GetActorLocation();
	Dir.Z = 0.f;
	if (!Dir.IsNearlyZero())
	{
		SetActorRotation(Dir.Rotation());
	}

	LastHeardNoiseTime = World->GetTimeSeconds();
	AddRage(NoiseRageGain);

	if (bCanShriek)
	{
		// ── 1) 사운드 (멀티캐스트로)
		if (SonicShriekSound)
		{
			Multicast_PlaySonicShriekSound(GetActorLocation());
		}

		// ── 2) 디버그 구
		DrawDebugSphere(World, GetActorLocation(), ShriekRadius, 32, FColor::Red, false, 2.f);


		AController* C = GetController();
		// ApplyRadialDamage는 Controller가 null이어도 동작하나, 안정성을 위해 체크
		UGameplayStatics::ApplyRadialDamage(
			this,
			ShriekDamage,
			GetActorLocation(),
			ShriekRadius,
			nullptr,
			TArray<AActor*>(),        // 빈 리스트
			this,
			C,
			true
		);


		bCanShriek = false;
		World->GetTimerManager().SetTimer(
			ShriekTimerHandle,
			this, &ALCBossBanshee::ResetShriek,
			ShriekCooldown,
			false
		);
	}
}

void ALCBossBanshee::Multicast_PlaySonicShriekSound_Implementation(const FVector& Location)
{
	if (!SonicShriekSound) return;

	UGameplayStatics::PlaySoundAtLocation(
		this,
		SonicShriekSound,
		Location,
		/*Volume=*/1.f,
		/*Pitch=*/1.f,
		/*StartTime=*/0.f,
		SonicShriekAttenuation   // 감쇠 세팅
	);
}

void ALCBossBanshee::ResetShriek()
{
	bCanShriek = true;
}

void ALCBossBanshee::AddRage(float Amount)
{
	if (!HasAuthority())
		return;

	// 0) MaxRage 유효성 검사
	if (MaxRage <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[Lich::AddRage] MaxRage 값이 유효하지 않습니다: %f"), MaxRage);
		return;
	}

	// 1) Rage 갱신
	float Mult = bIsBerserk ? RageGainMultiplier_Berserk : 1.f;
	Rage = FMath::Clamp(Rage + Amount * Mult, 0.f, MaxRage);

	// 2) 서버 권한이 있을 때만 Blackboard 업데이트

	UpdateBlackboardValues();


	// 3) 광폭화
	if (Rage >= MaxRage && !bIsBerserk)
	{
		StartBerserk(BerserkDuration);
		UpdateBlackboardValues();
	}
}

void ALCBossBanshee::DecayRage(float DeltaTime)
{
	if (GetWorld()->GetTimeSeconds() - LastHeardNoiseTime >= 15.f)
		AddRage(-RageDecayPerSecond * DeltaTime);
}

void ALCBossBanshee::Wail()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Origin = GetActorLocation();

	// (1) 사운드 → 멀티캐스트로
	if (WailSound)
	{
		Multicast_PlayWailSound(Origin);
	}

	// (2) SweepMultiByObjectType로 물리 판정
	TArray<FHitResult> HitResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(WailRange);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (World->SweepMultiByObjectType(
		HitResults,
		Origin, Origin,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		Sphere,
		Params))
	{
		// 서버에서만 데미지를 계산
		if (HasAuthority())
		{
			for (const FHitResult& Hit : HitResults)
			{
				if (ABaseCharacter* Target = Cast<ABaseCharacter>(Hit.GetActor()))
				{
					// (2-1) 넉백
					FVector Dir = (Target->GetActorLocation() - Origin).GetSafeNormal();
					FVector LaunchVelocity = Dir * 800.f + FVector(0, 0, 300.f);
					Target->LaunchCharacter(LaunchVelocity, true, true);

					// (2-2) 데미지 적용
					UGameplayStatics::ApplyDamage(
						Target,
						WailDamage,
						GetController(),
						this,
						nullptr
					);

					// (2-3) 정신력 감소
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Banshee] Wail triggered with HitResult sweep and damage"));
}

void ALCBossBanshee::Multicast_PlayWailSound_Implementation(const FVector& Location)
{
	// FX
	if (WailFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this->GetWorld(),
			WailFX,
			Location,
			FRotator::ZeroRotator
		);
	}

	if (WailSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WailSound,
			Location,
			/*Volume*/1.f,
			/*Pitch*/1.f,
			/*StartTime*/0.f,
			AttackAttenuation  // 감쇠 세팅
		);
	}


}

void ALCBossBanshee::EchoSlash()
{
	if (LastPingedLocation.IsZero())
		return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 1) FX (텔레포트 전)
	if (EchoSlashFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			EchoSlashFX,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}

	// 2) 목표 지점의 바닥 Z 보정
	FVector Desired = LastPingedLocation;
	{
		constexpr float TraceUp = 500.f;
		constexpr float TraceDown = 1000.f;
		FVector Start = Desired + FVector(0, 0, TraceUp);
		FVector End = Desired - FVector(0, 0, TraceDown);
		FHitResult Hit;
		FCollisionQueryParams Params(NAME_None, false, this);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			Desired.Z = Hit.Location.Z;
		}
	}

	// 3) Capsule 반높이만큼 올려서 땅에 파묻히지 않게
	if (UCapsuleComponent* Cap = FindComponentByClass<UCapsuleComponent>())
	{
		Desired.Z += Cap->GetScaledCapsuleHalfHeight();
	}

	// 4) 실제 텔레포트 (충돌 무시)
	SetActorLocation(Desired, false, nullptr, ETeleportType::TeleportPhysics);

	// 5) 데미지 적용
	UGameplayStatics::ApplyRadialDamage(
		this,
		EchoSlashDamage,
		Desired,
		400.f,
		nullptr,
		TArray<AActor*>(),
		this,
		GetController(),
		true
	);

	UE_LOG(LogTemp, Log, TEXT("[Banshee] EchoSlash executed at %s"), *Desired.ToString());
}

void ALCBossBanshee::DesperateWail()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Origin = GetActorLocation();

	// 1) 시각·음향 효과 (멀티캐스트)
	Multicast_PlayDesperateWailEffects(Origin);

	// 2) 맵상의 모든 플레이어에게 데미지 및 디버프
	const float DamageAmount = DesperateWailDamage;  // UPROPERTY로 노출된 값
	TArray<APlayerController*> LocalPCs;

	// 모든 플레이어 컨트롤러를 순회
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (APawn* Pawn = PC->GetPawn())
			{

				// 2‑2) 공포 디버프
				if (auto* Target = Cast<ABaseCharacter>(Pawn))
				{
					// 2‑1) 일반 대미지 적용
					UGameplayStatics::ApplyDamage(
						Pawn,
						DamageAmount,
						GetController(),
						this,
						UDamageType::StaticClass()
					);

					Multicast_ApplyFear(Target);

					// 2‑3) 슬로우 디버프
					if (UCharacterMovementComponent* MoveComp = Target->GetCharacterMovement())
					{
						float OrigSpeed = MoveComp->MaxWalkSpeed;
						MoveComp->MaxWalkSpeed = OrigSpeed * SlowMultiplier;

						FTimerHandle TimerHandle;
						World->GetTimerManager().SetTimer(
							TimerHandle,
							FTimerDelegate::CreateLambda([MoveComp, OrigSpeed]() {
								if (MoveComp) MoveComp->MaxWalkSpeed = OrigSpeed;
								}),
							SlowDuration,
							false
						);
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Banshee] DesperateWail executed: all players damaged"));
}

void ALCBossBanshee::Multicast_PlayDesperateWailEffects_Implementation(const FVector& Origin)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// FX
	if (DesperateWailFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			DesperateWailFX,
			Origin,
			FRotator::ZeroRotator
		);
	}

	// Sound (전 범위에서 들리게, attenuation 없이)
	if (DesperateWailSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DesperateWailSound,
			Origin
		);
	}
}

// --- Multicast RPC 구현 ---
void ALCBossBanshee::Multicast_ApplyFear_Implementation(ACharacter* Target)
{
	// 0) 월드 & 타겟 유효성 검사
	UWorld* World = GetWorld();
	if (!World || !IsValid(Target))
	{
		return;
	}

	// 1) 로컬 플레이어 클라이언트에만 포스트프로세스 적용
	APlayerController* PC = Cast<APlayerController>(Target->GetController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// 2) 카메라 컴포넌트 획득 및 유효성 검사
	UCameraComponent* Cam = Target->FindComponentByClass<UCameraComponent>();
	if (!Cam || !FearPostProcessMat)
	{
		return;
	}

	// 3) 포스트프로세스 머티리얼 추가
	Cam->PostProcessSettings.AddBlendable(FearPostProcessMat, FearPostProcessWeight);

	// 4) 일정 시간 후 Blendable 제거
	FTimerHandle RemoveHandle;
	FTimerDelegate RestoreDel = FTimerDelegate::CreateLambda([Cam, this]()
		{
			if (Cam && FearPostProcessMat)
			{
				Cam->PostProcessSettings.RemoveBlendable(FearPostProcessMat);
			}
		});
	World->GetTimerManager().SetTimer(RemoveHandle, RestoreDel, FearDuration, false);
}

void ALCBossBanshee::SpawnBansheeClones()
{
	for (int32 i = 0; i < CloneCount; ++i)
	{
		float Angle = (360.f / CloneCount) * i;
		FVector Offset = FVector(FMath::Cos(FMath::DegreesToRadians(Angle)), FMath::Sin(FMath::DegreesToRadians(Angle)), 0.f) * CloneSpawnRadius;
		FVector SpawnLocation = GetActorLocation() + Offset;

		if (CloneSpawnFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), CloneSpawnFX, SpawnLocation, FRotator::ZeroRotator);
		}

		// 실제 분신은 별도 ACharacter 서브클래스에서 스폰
		GetWorld()->SpawnActor<ACloneMinion>(CloneClass, SpawnLocation, FRotator::ZeroRotator);
	}

	UE_LOG(LogTemp, Log, TEXT("[Banshee] %d Clones Spawned"), CloneCount);
}

bool ALCBossBanshee::RequestAttack(float TargetDistance)
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

	struct FAttackEntry { float Weight, Range; TFunction<void()> Action; };
	TArray<FAttackEntry> Entries;

	// (1) EchoSlash 후보 수집 (쿨타임·거리만 체크)
	const float EchoSlashRange = 800.f;  // 원하는 범위 변수로 대체 가능
	if (!LastPingedLocation.IsZero()
		&& TargetDistance > EchoSlashRange
		&& Now - LastEchoSlashTime >= EchoSlashCooldown)
	{
		Entries.Add({
			EchoSlashWeight,
			EchoSlashRange,
			[this, Now]() {
				LastEchoSlashTime = Now;
				UE_LOG(LogTemp, Warning, TEXT("[Banshee] Echo Slash 발동"));
				EchoSlash();
			}
			});
	}

	// (2) Wail 후보 수집
	if (Now - LastWailTime >= WailCooldown)
	{
		Entries.Add({
			WailWeight,
			WailRange,
			[this, Now]() {
				LastWailTime = Now;
				UE_LOG(LogTemp, Warning, TEXT("[Banshee] Wail 발동"));
				Wail();
			}
			});
	}

	// 3) 후보가 없으면 선택 실패
	if (Entries.Num() == 0)
		return false;

	// 4) 가중치 랜덤 선택
	float TotalW = 0.f;
	for (auto& E : Entries) TotalW += E.Weight;

	float Pick = FMath::FRandRange(0.f, TotalW), Acc = 0.f;
	for (auto& E : Entries)
	{
		Acc += E.Weight;
		if (Pick <= Acc)
		{
			// 선택된 스킬의 사거리와 액션만 저장
			NextAttackRange = E.Range;
			NextAttackAction = E.Action;
			return true;  // 선택만 하고 즉시 공격은 하지 않음
		}
	}

	return false;
}
