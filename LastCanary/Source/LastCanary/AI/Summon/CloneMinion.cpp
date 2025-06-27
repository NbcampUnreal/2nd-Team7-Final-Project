#include "AI/Summon/CloneMinion.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Controller.h"

ACloneMinion::ACloneMinion()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ACloneMinion::BeginPlay()
{
    Super::BeginPlay();

    // 일정 시간 후 자동 자폭
    GetWorldTimerManager().SetTimer(
        SelfDestructHandle,
        this,
        &ACloneMinion::SelfDestruct,
        Lifetime,
        false
    );
}

void ACloneMinion::SelfDestruct()
{
    // 먼저 모든 클라이언트에 이펙트와 사운드 재생 RPC
    Multicast_PlayExplosionEffects();

    // 서버에서만 대미지 처리
    if (HasAuthority())
    {
        UGameplayStatics::ApplyRadialDamage(
            this,
            ExplosionDamage,
            GetActorLocation(),
            ExplosionRadius,
            nullptr,
            TArray<AActor*>(),
            this,
            GetController(),
            true
        );

        Destroy();
    }
}

void ACloneMinion::Multicast_PlayExplosionEffects_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 이펙트
    if (DisappearEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            DisappearEffect,
            GetActorLocation(),
            FRotator::ZeroRotator,
            FVector(1.f),  // Scale
            true           // AutoDestroy
        );
    }

    // 사운드 (감쇠 적용)
    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ExplosionSound,
            GetActorLocation(),
            1.f,         // Volume
            1.f,         // Pitch
            0.f,         // StartTime
            ExplosionAttenuation  // Attenuation 설정
        );
    }
}