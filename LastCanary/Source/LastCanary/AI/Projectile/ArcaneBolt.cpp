#include "AI/Projectile/ArcaneBolt.h"
#include "GameFramework/ProjectileMovementComponent.h"  // ← 이 줄 추가
#include "Kismet/GameplayStatics.h"

AArcaneBolt::AArcaneBolt()
{
    // ArcaneBolt-specific 초기값 세팅(속도, 데미지 등)
    MovementComp->InitialSpeed = 1200.f;
    MovementComp->MaxSpeed = 1200.f;
    ProjectileDamage = 20.f;
}

void AArcaneBolt::InitProjectile(const FVector& Direction, float Speed, float Damage, AController* InstigatorController)
{
    // 1) 속도 설정
    if (MovementComp)
    {
        MovementComp->Velocity = Direction.GetSafeNormal() * Speed;
    }

    // 2) 데미지 설정
    ProjectileDamage = Damage;

    // 3) 인스티게이터(공격 주체) 지정
    InstigatorControllerRef = InstigatorController;
    SetInstigator(InstigatorController ? InstigatorController->GetPawn() : nullptr);

    // (선택) 스폰 시 추가 FX/사운드
    if (SpawnFX)
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SpawnFX, GetActorLocation());
    if (SpawnSound)
        UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
}