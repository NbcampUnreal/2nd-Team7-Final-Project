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
    if (DisappearEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), DisappearEffect, GetActorLocation());
    }

    UGameplayStatics::ApplyRadialDamage(
        this,
        ExplosionDamage,
        GetActorLocation(),
        ExplosionRadius,
        nullptr,
        {},
        this,
        GetController(),
        true
    );

    Destroy();
}