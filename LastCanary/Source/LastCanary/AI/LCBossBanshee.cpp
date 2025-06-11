﻿#include "AI/LCBossBanshee.h"
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
	if (bIsBerserk)
	{
        // 예: 반향 시마다 추가로 느리게 만드는 디버프를 걸거나, 파티클을 뿌리는 로직
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            PingRadius * 1.2f,
            16,
            FColor::Purple,
            false,
            1.0f
        );
	}

    else
    {
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            PingRadius,
            32,
            FColor::Cyan,
            false,
            RevealDuration
        );
    }
    


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

void ALCBossBanshee::BerserkExtraEcho()
{

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

        // ── Berserk 시 추가 폭발 효과 ──
        if (bIsBerserk)
        {
            // 예: 강력한 한 번 더 ApplyRadialDamage 하거나, 폭발 파티클
            UGameplayStatics::ApplyRadialDamage(
                this,
                ShriekDamage * 0.5f,
                GetActorLocation(),
                ShriekRadius * 1.5f,
                nullptr,
                TArray<AActor*>(),
                this,
                GetController(),
                true
            );
            DrawDebugSphere(
                GetWorld(),
                GetActorLocation(),
                ShriekRadius * 1.5f,
                24,
                FColor::Orange,
                false,
                2.f
            );
            UE_LOG(LogTemp, Log, TEXT("[Banshee] BerserkExtraShriek 실행"));
        }


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
