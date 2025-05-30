#include "AI/LCBossBanshee.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Character.h"

ALCBossBanshee::ALCBossBanshee()
{
    PrimaryActorTick.bCanEverTick = true;
    // bCanShriek는 선언 시 초기화
}

void ALCBossBanshee::BeginPlay()
{
    Super::BeginPlay();

    // 주기적인 반향 정찰 시작
    GetWorldTimerManager().SetTimer(
        PingTimerHandle,
        this,
        &ALCBossBanshee::EcholocationPing,
        PingInterval,
        true
    );
}

void ALCBossBanshee::EcholocationPing()
{
    // 1) 반향 소리 재생
    if (EcholocationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, EcholocationSound, GetActorLocation());
    }

    // 2) 디버그용 반경 시각화
    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        PingRadius,
        32,
        FColor::Cyan,
        false,
        RevealDuration
    );

    // 3) 반경 내 모든 플레이어 캐릭터 표시
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(PingRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        Sphere,
        Params
    );

    if (bHit)
    {
        for (auto& Result : Overlaps)
        {
            if (ACharacter* Char = Cast<ACharacter>(Result.GetActor()))
            {
                // 예시: 가시성 토글
                Char->SetActorHiddenInGame(false);
                // RevealDuration 이후 다시 숨기려면 별도 로직 필요
            }
        }
    }
}

void ALCBossBanshee::OnHeardNoise(const FVector& NoiseLocation)
{
    // 1) 소리 발생 지점 바라보기
    FVector Dir = NoiseLocation - GetActorLocation();
    Dir.Z = 0.f;
    if (!Dir.IsNearlyZero())
    {
        SetActorRotation(Dir.Rotation());
    }

    // 2) 울부짖기
    if (bCanShriek)
    {
        // 사운드 재생
        if (SonicShriekSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this, SonicShriekSound, GetActorLocation());
        }

        // 반경 데미지 적용
        UGameplayStatics::ApplyRadialDamage(
            this,
            ShriekDamage,
            GetActorLocation(),
            ShriekRadius,
            nullptr,
            TArray<AActor*>(),
            this,
            GetController(),
            true
        );

        // 디버그 시각화
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            ShriekRadius,
            32,
            FColor::Red,
            false,
            2.f
        );

        // 쿨다운 시작
        bCanShriek = false;
        GetWorldTimerManager().SetTimer(
            ShriekTimerHandle,
            this,
            &ALCBossBanshee::ResetShriek,
            ShriekCooldown,
            false
        );
    }
}

void ALCBossBanshee::ResetShriek()
{
    bCanShriek = true;
}
