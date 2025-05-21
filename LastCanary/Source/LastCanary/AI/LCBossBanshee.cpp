#include "LCBossBanshee.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

ALCBossBanshee::ALCBossBanshee()
{
    PrimaryActorTick.bCanEverTick = true;

    // 기본 값 설정
    PingInterval = 8.f;
    PingRadius = 2000.f;
    RevealDuration = 4.f;
    ShriekDamage = 75.f;
    ShriekRadius = 800.f;
    ShriekCooldown = 6.f;
    bCanShriek = true;
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
    // 반향 소리 재생
    if (EcholocationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, EcholocationSound, GetActorLocation());
    }

    // 디버그용 반경 시각화
    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        PingRadius,
        32,
        FColor::Cyan,
        false,
        RevealDuration
    );

    // 반경 내 모든 플레이어 캐릭터 표시
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(PingRadius);
    if (GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
        Sphere))
    {
        for (auto& Overlap : Overlaps)
        {
            AActor* Actor = Overlap.GetActor();
            if (Actor && Actor->IsA(ACharacter::StaticClass()))
            {
                // 예시: 가시성 토글 또는 효과 적용
                Actor->SetActorHiddenInGame(false);
                // RevealDuration 동안 추가 효과를 적용할 수 있습니다.
            }
        }
    }
}

void ALCBossBanshee::OnHeardNoise(const FVector& NoiseLocation)
{
    // 소리 발생 지점을 바라보도록 회전
    FVector Dir = NoiseLocation - GetActorLocation();
    Dir.Z = 0.f;
    if (!Dir.IsNearlyZero())
    {
        FRotator NewRot = Dir.Rotation();
        SetActorRotation(NewRot);
    }

    // 사용 가능 시 소닉 울부짖기 수행
    if (bCanShriek)
    {
        // 울부짖는 소리 재생
        if (SonicShriekSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, SonicShriekSound, GetActorLocation());
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

        // 디버그용 시각화
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            ShriekRadius,
            32,
            FColor::Red,
            false,
            2.f
        );

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
    // bCanShriek 다시 허용
    bCanShriek = true;
}
