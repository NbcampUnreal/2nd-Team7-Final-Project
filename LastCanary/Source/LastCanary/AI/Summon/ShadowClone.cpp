#include "AI/Summon/ShadowClone.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

AShadowClone::AShadowClone()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // 분신은 플레이어 공격을 받지 않으므로 간단히 작은 콜리전만 남김
    GetCapsuleComponent()->SetCapsuleRadius(30.f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("OverlapAllPawn"));
}

void AShadowClone::BeginPlay()
{
    Super::BeginPlay();

    // 머티리얼 다이내믹 인스턴스 생성 (페이드 아웃용)
    if (CloneMaterialBase && GetMesh())
    {
        DynMaterial = UMaterialInstanceDynamic::Create(CloneMaterialBase, this);
        GetMesh()->SetMaterial(0, DynMaterial);
    }

    // 일정 시간 뒤 페이드 아웃
    GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &AShadowClone::FadeOutAndDestroy, LifeTime, false);
}

void AShadowClone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 간단히 보스 위치를 따라다니도록
    if (AActor* OwnerActor = GetOwner())
    {
        FVector Direction = (OwnerActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
}

float AShadowClone::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    Health -= DamageAmount;
    if (Health <= 0.f)
    {
        FadeOutAndDestroy();
    }
    return DamageAmount;
}

void AShadowClone::FadeOutAndDestroy()
{
    // 페이드 아웃 애니메이션: 머티리얼 파라미터 알파를 줄인다
    if (DynMaterial)
    {
        // 알파를 0으로 천천히 변경
        DynMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.f);
    }

    // 파괴 예약 (페이드아웃 시간 후)
    GetWorldTimerManager().ClearTimer(LifeTimerHandle);
    GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &AShadowClone::K2_DestroyActor, FadeOutTime, false);
}