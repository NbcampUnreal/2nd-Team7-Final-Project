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
    DecayRage(DeltaTime);

    if (auto* AICon = Cast<ALCBaseBossAIController>(GetController()))
    {
        if (auto* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsFloat(TEXT("RagePercent"), Rage / MaxRage);
            BB->SetValueAsBool(TEXT("IsBerserkMode"), bIsBerserk);
        }
    }
}

void ALCBossBanshee::EcholocationPing()
{
    if (EcholocationSound)
        UGameplayStatics::PlaySoundAtLocation(this, EcholocationSound, GetActorLocation());

    const float Radius = bIsBerserk ? PingRadius * 1.2f : PingRadius;
    const FVector Origin = GetActorLocation();

    DrawDebugSphere(GetWorld(), Origin, Radius, 32, FColor::Cyan, false, RevealDuration);

    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bRevealedAnyone = false;
    bool bFoundPingTarget = false;

    if (GetWorld()->SweepMultiByObjectType(HitResults, Origin, Origin, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn), Sphere, Params))
    {
        for (const FHitResult& Hit : HitResults)
        {
            ACharacter* HitCharacter = Cast<ACharacter>(Hit.GetActor());
            if (!HitCharacter)
                continue;

            if (USkeletalMeshComponent* SkeletalMesh = HitCharacter->GetMesh())
            {
                SkeletalMesh->SetRenderCustomDepth(true);
                SkeletalMesh->SetCustomDepthStencilValue(252); // 강조용 스텐실 값
            }

            HandleRehide(HitCharacter);
            bRevealedAnyone = true;

            if (!bFoundPingTarget)
            {
                LastPingedLocation = HitCharacter->GetActorLocation();
                bFoundPingTarget = true;
            }
        }
    }

    AddRage(bRevealedAnyone ? 5.f : -5.f);
}

void ALCBossBanshee::EnterBerserkState()
{
    Super::EnterBerserkState();
    UE_LOG(LogTemp, Warning, TEXT("[Banshee] 영구 Berserk 진입"));
}

void ALCBossBanshee::StartBerserk()
{
    Super::StartBerserk();  // bIsBerserk = true 및 Multicast 호출 포함

    // 이펙트 추가
    if (BerserkEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            BerserkEffectFX,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }

}

void ALCBossBanshee::StartBerserk(float Duration)
{
    Super::StartBerserk(Duration);

    // 동일 이펙트 재생
    if (BerserkEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            BerserkEffectFX,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }


}

void ALCBossBanshee::EndBerserk()
{
    Super::EndBerserk();

    UE_LOG(LogTemp, Warning, TEXT("[Banshee] Berserk 종료"));



    // 종료 시 후처리가 필요하다면 여기에
}

void ALCBossBanshee::OnRep_IsBerserk()
{
    Super::OnRep_IsBerserk();

    if (bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Banshee] OnRep → Berserk 이펙트 클라에서 재생"));

        if (BerserkEffectFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                BerserkEffectFX,
                GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }
    }
}

void ALCBossBanshee::HandleRehide(ACharacter* Char)
{
    FTimerHandle TempHandle;
    GetWorldTimerManager().SetTimer(TempHandle, [Char]()
        {
            if (IsValid(Char))
            {
                if (USkeletalMeshComponent* Mesh = Char->GetMesh())
                    Mesh->SetRenderCustomDepth(false);
            }
        }, RevealDuration, false);
}

void ALCBossBanshee::OnHeardNoise(const FVector& NoiseLocation)
{
    FVector Dir = NoiseLocation - GetActorLocation();
    Dir.Z = 0.f;
    if (!Dir.IsNearlyZero())
        SetActorRotation(Dir.Rotation());

    LastHeardNoiseTime = GetWorld()->GetTimeSeconds();
    AddRage(10.f);

    if (bCanShriek)
    {
        if (SonicShriekSound)
            UGameplayStatics::PlaySoundAtLocation(this, SonicShriekSound, GetActorLocation());

        DrawDebugSphere(GetWorld(), GetActorLocation(), ShriekRadius, 32, FColor::Red, false, 2.f);

        if (bIsBerserk)
        {
            UGameplayStatics::ApplyRadialDamage(
                this,
                ShriekDamage * 0.5f,
                GetActorLocation(),
                ShriekRadius * 1.5f,
                nullptr,
                {},
                this,
                GetController(),
                true
            );
        }

        bCanShriek = false;
        GetWorldTimerManager().SetTimer(ShriekTimerHandle, this, &ALCBossBanshee::ResetShriek, ShriekCooldown, false);
    }
}

void ALCBossBanshee::ResetShriek()
{
    bCanShriek = true;
}

void ALCBossBanshee::AddRage(float Amount)
{
    float Multiplier = bIsBerserk ? RageGainMultiplier_Berserk : 1.f;
    Rage = FMath::Clamp(Rage + Amount * Multiplier, 0.f, MaxRage);

    if (Rage >= MaxRage && !bIsBerserk)
    {
        StartBerserk(BerserkDuration);
        MulticastActivateBerserkEffects();
    }
}

void ALCBossBanshee::DecayRage(float DeltaTime)
{
    if (GetWorld()->GetTimeSeconds() - LastHeardNoiseTime >= 15.f)
        AddRage(-RageDecayPerSecond * DeltaTime);
}

void ALCBossBanshee::MulticastActivateBerserkEffects_Implementation()
{
    if (BerserkEffectFX)
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            BerserkEffectFX,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );

    UE_LOG(LogTemp, Log, TEXT("[Banshee] Berserk Activated"));
}

void ALCBossBanshee::Wail()
{
    if (WailSound)
        UGameplayStatics::PlaySoundAtLocation(this, WailSound, GetActorLocation());

    const FVector Origin = GetActorLocation();
    const float Radius = WailRange;

    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->SweepMultiByObjectType(
        HitResults,
        Origin,
        Origin,
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        Sphere,
        Params))
    {
        for (const FHitResult& Hit : HitResults)
        {
            ACharacter* Target = Cast<ACharacter>(Hit.GetActor());
            if (!Target) continue;

            FVector Dir = (Target->GetActorLocation() - Origin).GetSafeNormal();
            FVector LaunchVelocity = Dir * 800.f + FVector(0, 0, 300.f);
            Target->LaunchCharacter(LaunchVelocity, true, true);

            // 공포 디버프 적용
            // Target->ApplyDebuff("Fear", 3.0f);
        }
    }

    if (WailFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            WailFX,
            Origin,
            FRotator::ZeroRotator
        );
    }

    UE_LOG(LogTemp, Log, TEXT("[Banshee] Wail triggered with HitResult sweep"));
}

void ALCBossBanshee::EchoSlash()
{
    if (!LastPingedLocation.IsZero())
    {
        if (EchoSlashFX)
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EchoSlashFX, GetActorLocation());

        SetActorLocation(LastPingedLocation, false, nullptr, ETeleportType::TeleportPhysics);

        TArray<AActor*> Ignored;
        UGameplayStatics::ApplyRadialDamage(
            this,
            60.f,
            GetActorLocation(),
            400.f,
            nullptr,
            Ignored,
            this,
            GetController(),
            true
        );

        UE_LOG(LogTemp, Log, TEXT("[Banshee] EchoSlash executed at %s"), *LastPingedLocation.ToString());
    }
}

void ALCBossBanshee::DesperateWail()
{
    // 1) 사운드
    if (DesperateWailSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            DesperateWailSound,
            GetActorLocation(),
            1.f,               // Volume
            1.f,               // Pitch
            0.f,               // StartTime
            DesperateWailAttenuation  // 감쇠 에셋
        );
    }

    // 2) SweepMultiByObjectType 로 모든 Pawn 검색
    const float MapRadius = 10000.f;
    const FVector Origin = GetActorLocation();
    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(MapRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->SweepMultiByObjectType(
        HitResults, Origin, Origin, FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn), Sphere, Params))
    {
        for (const FHitResult& Hit : HitResults)
        {
            ABaseCharacter* Target = Cast<ABaseCharacter>(Hit.GetActor());
            if (!Target) continue;

            // --- 공포 디버프: Multicast 로 클라이언트 연출 ---
            Multicast_ApplyFear(Target);

            // --- 슬로우 디버프: 서버에서 즉시 적용 & 타이머로 원복 ---
            if (auto* MoveComp = Target->GetCharacterMovement())
            {
                float OrigSpeed = MoveComp->MaxWalkSpeed;
                MoveComp->MaxWalkSpeed = OrigSpeed * SlowMultiplier;

                // 일정 시간 후 원복
                FTimerHandle TimerHandle;
                FTimerDelegate RestoreDel = FTimerDelegate::CreateLambda(
                    [MoveComp, OrigSpeed]()
                    {
                        if (MoveComp) MoveComp->MaxWalkSpeed = OrigSpeed;
                    });
                GetWorldTimerManager().SetTimer(TimerHandle, RestoreDel, SlowDuration, false);
            }
        }
    }

    // 3) FX
    if (DesperateWailFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), DesperateWailFX,
            Origin, FRotator::ZeroRotator);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Banshee] DesperateWail executed"));
}

// --- Multicast RPC 구현 ---
void ALCBossBanshee::Multicast_ApplyFear_Implementation(ACharacter* Target)
{
    if (!Target)
        return;

    // 로컬 플레이어 클라이언트에만 포스트프로세스 적용
    if (APlayerController* PC = Cast<APlayerController>(Target->GetController()))
    {
        if (!PC->IsLocalController())
            return;

        // Character에 붙은 CameraComponent의 PostProcessSettings 사용
        if (UCameraComponent* Cam = Target->FindComponentByClass<UCameraComponent>())
        {
            // 포스트프로세스 머티리얼 추가
            Cam->PostProcessSettings.AddBlendable(FearPostProcessMat, FearPostProcessWeight);

            // 일정 시간 후 Blendable 제거
            FTimerHandle RemoveHandle;
            GetWorldTimerManager().SetTimer(RemoveHandle, FTimerDelegate::CreateLambda(
                [Cam, this]()
                {
                    if (Cam)
                        Cam->PostProcessSettings.RemoveBlendable(FearPostProcessMat);
                }),
                FearDuration,
                false
            );
        }
    }
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
        // GetWorld()->SpawnActor<ACloneMinion>(CloneClass, SpawnLocation, FRotator::ZeroRotator);
    }

    UE_LOG(LogTemp, Log, TEXT("[Banshee] %d Clones Spawned"), CloneCount);
}

bool ALCBossBanshee::RequestAttack(float TargetDistance)
{
    if (!HasAuthority()) return false;

    const float Now = GetWorld()->GetTimeSeconds();
    struct FAttackEntry { float Weight; TFunction<void()> Action; };
    TArray<FAttackEntry> Entries;

    // (1) 특수 스킬 - Desperate Wail
    if (!bHasUsedDesperateWail)
    {
        bHasUsedDesperateWail = true;
        UE_LOG(LogTemp, Log, TEXT("[Banshee] Desperate Wail 발동"));
        DesperateWail();
        return true;
    }

    // (2) EchoSlash - 최근 핑된 위치로 순간이동 후 공격
    if (!LastPingedLocation.IsZero() && TargetDistance > 800.f && Now - LastEchoSlashTime >= EchoSlashCooldown)
    {
        Entries.Add({ EchoSlashWeight, [this, Now]()
        {
            LastEchoSlashTime = Now;
            UE_LOG(LogTemp, Log, TEXT("[Banshee] Echo Slash 발동"));
            EchoSlash();
        } });
    }

    // (3) Wail - 근접 범위 울부짖기 공격
    if (TargetDistance <= WailRange && Now - LastWailTime >= WailCooldown)
    {
        Entries.Add({ WailWeight, [this, Now]()
        {
            LastWailTime = Now;
            UE_LOG(LogTemp, Log, TEXT("[Banshee] Wail 발동"));
            Wail();
        } });
    }

    // 가중치 랜덤 선택
    float TotalWeight = 0.f;
    for (auto& Entry : Entries) TotalWeight += Entry.Weight;
    if (TotalWeight <= 0.f) return false;

    float Pick = FMath::FRandRange(0.f, TotalWeight);
    float Acc = 0.f;
    for (auto& Entry : Entries)
    {
        Acc += Entry.Weight;
        if (Pick <= Acc)
        {
            Entry.Action();
            return true;
        }
    }

    return false;
}
