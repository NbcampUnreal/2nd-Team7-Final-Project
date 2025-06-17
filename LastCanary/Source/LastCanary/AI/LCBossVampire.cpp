#include "AI/LCBossVampire.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/BaseCharacter.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"

ALCBossVampire::ALCBossVampire()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ALCBossVampire::BeginPlay()
{
    Super::BeginPlay();

    // 배트 스웜 & 악몽의 시선
    GetWorldTimerManager().SetTimer(BatSwarmTimerHandle, this, &ALCBossVampire::ExecuteBatSwarm, BatSwarmInterval, true);
    GetWorldTimerManager().SetTimer(GazeTimerHandle, this, &ALCBossVampire::ExecuteNightmareGaze, GazeInterval, true);
}

void ALCBossVampire::ExecuteBatSwarm()
{
    if (!HasAuthority() || !BatSwarmClass) return;
    for (int32 i = 0; i < BatCount; ++i)
    {
        FVector Dir = FMath::VRand();
        Dir.Z = 0.5f;
        FVector Loc = GetActorLocation() + Dir * 300.f + FVector(0, 0, 100);
        GetWorld()->SpawnActor<AActor>(BatSwarmClass, Loc, Dir.Rotation());
    }
}

void ALCBossVampire::ExecuteNightmareGaze()
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] NightmareGaze"));

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(GazeRadius);

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        GazeRadius,
        16,
        FColor::Purple,
        false,
        2.0f,
        0,
        5.0f
    );


    if (GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& H : Hits)
        {
            if (ACharacter* Ch = Cast<ABaseCharacter>(H.GetActor()))
            {
                if (Ch->IsPlayerControlled())
                {
                    float Orig = Ch->GetCharacterMovement()->MaxWalkSpeed;
                    Ch->GetCharacterMovement()->MaxWalkSpeed = Orig * 0.5f;
                    //  데미지 없음, 디버프만
                    FTimerHandle Tmp;
                    FTimerDelegate D = FTimerDelegate::CreateLambda([Ch, Orig]() { Ch->GetCharacterMovement()->MaxWalkSpeed = Orig; });
                    GetWorldTimerManager().SetTimer(Tmp, D, GazeDebuffDuration, false);
                }
            }
        }
    }
}

void ALCBossVampire::ExecuteCrimsonSlash()
{
    GetWorldTimerManager().SetTimer(CrimsonSlashHandle, CrimsonSlashCooldown, false);

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] CrimsonSlash"));

    TArray<FHitResult> Hits;
    FCollisionShape S = FCollisionShape::MakeSphere(CrimsonSlashRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, S))
    {
        float Heal = CrimsonSlashDamage * BloodDrainEfficiency;
        for (auto& H : Hits)
        {
            if (ACharacter* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                UGameplayStatics::ApplyDamage(C, CrimsonSlashDamage, GetController(), this, nullptr);
            }
        }
        // 피흡
        UGameplayStatics::ApplyDamage(this, -Heal, GetController(), this, nullptr);
    }
}

void ALCBossVampire::ExecuteSanguineBurst()
{
    GetWorldTimerManager().SetTimer(BurstHandle, SanguineBurstCooldown, false);

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] SanguineBurst"));

    TArray<FHitResult> Hits;
    FCollisionShape S = FCollisionShape::MakeSphere(SanguineBurstRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, S))
    {
        for (auto& H : Hits)
        {
            if (ACharacter* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                UGameplayStatics::ApplyDamage(C, SanguineBurstDamage, GetController(), this, nullptr);
            }
        }

    }
}

void ALCBossVampire::EnterMistForm()
{
    if (!HasAuthority() || !bCanUseMist) return;

    bCanUseMist = false;
    bIsMistForm = true;

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] Enter Mist Form"));

    Multicast_StartMistForm();

    SetCanBeDamaged(false);
    GetWorldTimerManager().SetTimer(MistDurationHandle, this, &ALCBossVampire::EndMistForm, MistDuration, false);
}

void ALCBossVampire::EndMistForm()
{
    bIsMistForm = false;
    OnRep_MistForm();

    // 무형 해제용 이펙트 & 사운드 재생
    if (MistExitEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            MistExitEffectFX,
            GetActorLocation(),
            GetActorRotation()
        );
    }
    if (MistExitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            MistExitSound,
            GetActorLocation()
        );
    }

    // 쿨다운 후 재사용 가능하도록
    GetWorldTimerManager().SetTimer(
        MistResetHandle,
        [this]() { bCanUseMist = true; },
        MistCooldown,
        false
    );
}

void ALCBossVampire::OnRep_MistForm()
{
    if (bIsMistForm)
    {
        // 1) 무형 진입 이펙트 재생 (Niagara)
        if (MistEnterEffectFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                MistEnterEffectFX,
                GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }

        // 2) 무형 진입 사운드 재생
        if (MistEnterSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                MistEnterSound,
                GetActorLocation()
            );
        }
    }
    else
    {
        // 1) 무형 해제 이펙트 재생
        if (MistExitEffectFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                MistExitEffectFX,
                GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }

        // 2) 무형 해제 사운드 재생
        if (MistExitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                MistExitSound,
                GetActorLocation()
            );
        }
    }
}

void ALCBossVampire::Multicast_StartMistForm_Implementation()
{
    OnRep_MistForm();
}

void ALCBossVampire::OnRep_Bloodlust()
{
    if (bIsBloodlust)
    {
        // Mist Form 즉시 사용 가능, 흡수 2배
        bCanUseMist = true;
        BloodDrainEfficiency = 2.0f;

        // 캐시
        OriginalMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;
        OriginalCrimsonCooldown = CrimsonSlashCooldown;
        OriginalBurstCooldown = SanguineBurstCooldown;

        // 속도 증가
        GetCharacterMovement()->MaxWalkSpeed *= 1.3f;
        // 쿨다운 감소
        CrimsonSlashCooldown *= 0.7f;
        SanguineBurstCooldown *= 0.7f;

        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Eternal Bloodlust Activated"));
    }
    else
    {
        // 복구
        BloodDrainEfficiency = 1.0f;
        GetCharacterMovement()->MaxWalkSpeed = OriginalMoveSpeed;
        CrimsonSlashCooldown = OriginalCrimsonCooldown;
        SanguineBurstCooldown = OriginalBurstCooldown;

        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Eternal Bloodlust Ended"));
    }
}

void ALCBossVampire::Multicast_StartBloodlust_Implementation()
{
    bIsBloodlust = true;
    OnRep_Bloodlust();

    // 지속시간 후 bIsBloodlust=false → OnRep_Bloodlust() 호출
    GetWorldTimerManager().ClearTimer(BloodlustDurationHandle);
    GetWorldTimerManager().SetTimer(BloodlustDurationHandle, [this]()
        {
            bIsBloodlust = false;
            OnRep_Bloodlust();
        }, BloodlustDuration, false);
}

// --- RepNotify 에서 클라이언트에서도 연출/해제 ---
void ALCBossVampire::OnRep_IsBerserk()
{
    Super::OnRep_IsBerserk();

    if (bIsBerserk)
        StartBerserk();
    else
        EndBerserk();
}

// --- 서버에서 Rage 가 Max 에 도달했을 때 최초 진입 (입장직후, Enter→Start 자동 실행) ---
void ALCBossVampire::EnterBerserkState()
{
    Super::EnterBerserkState();
    UE_LOG(LogTemp, Warning, TEXT("[Vampire] Enter Eternal Bloodlust"));

    // 이때 특수 상태 플래그 켜기 → 기존 Multicast_StartBloodlust 에서
    // Mist Form 즉시 재사용, 효율 2배, 속도↑ 등의 로직이 실행됩니다.
    Multicast_StartBloodlust();
}

// --- bIsBerserk=true 직후(클라이언트/서버 공통) 이펙트 & 사운드 ---
void ALCBossVampire::StartBerserk()
{
    Super::StartBerserk();

    // (1) 붉은 안개 FX
    if (BloodlustEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            BloodlustEffectFX,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }

    // (2) 심장 박동음 재생
    if (BloodlustSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            BloodlustSound,
            GetActorLocation()
        );
    }
}

// --- Duration 동안 유지하고 싶을 때 사용 (예: 20초) ---
void ALCBossVampire::StartBerserk(float Duration)
{
    // 기본 StartBerserk() 의 연출 재생
    StartBerserk();

    // Duration 초 뒤 EndBerserk() 호출 예약
    GetWorldTimerManager().ClearTimer(BerserkTimerHandle);
    GetWorldTimerManager().SetTimer(
        BerserkTimerHandle,
        this, &ALCBossVampire::EndBerserk,
        Duration, false
    );
}

// --- bIsBerserk=false 로 전환(클라이언트/서버) 시 실행 ---
void ALCBossVampire::EndBerserk()
{
    Super::EndBerserk();
    UE_LOG(LogTemp, Warning, TEXT("[Vampire] Exit Eternal Bloodlust"));

    // (선택) 추가 연출이 필요하면 여기에
    // 예: 붉은 안개 FX 제거, 사운드 정지 등
}

bool ALCBossVampire::RequestAttack(float TargetDistance)
{
    if (!HasAuthority()) return false;

    struct FEntry { float W; TFunction<void()> A; };
    TArray<FEntry> Entries;

    // Crimson Slash (근접)
    if (TargetDistance <= CrimsonSlashRadius && !GetWorldTimerManager().IsTimerActive(CrimsonSlashHandle))
        Entries.Add({ 3.f, [this]() { ExecuteCrimsonSlash(); } });

    // Sanguine Burst (중거리)
    if (TargetDistance <= SanguineBurstRadius && !GetWorldTimerManager().IsTimerActive(BurstHandle))
        Entries.Add({ 2.f, [this]() { ExecuteSanguineBurst(); } });

    // Mist Form (Fallback)
    if (bCanUseMist)
        Entries.Add({ 1.f, [this]() { EnterMistForm(); } });

    float Total = 0; for (auto& e : Entries) Total += e.W;
    float Pick = FMath::FRandRange(0.f, Total), Acc = 0;
    for (auto& e : Entries)
    {
        Acc += e.W;
        if (Pick <= Acc) { e.A(); return true; }
    }
    return false;
}

void ALCBossVampire::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossVampire, bIsMistForm);
    DOREPLIFETIME(ALCBossVampire, bIsBloodlust);
}