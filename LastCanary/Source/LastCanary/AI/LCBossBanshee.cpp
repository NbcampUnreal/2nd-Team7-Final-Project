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
    // bCanShriek�� ���� �� �ʱ�ȭ
}

void ALCBossBanshee::BeginPlay()
{
    Super::BeginPlay();

    // �ֱ����� ���� ���� ����
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
    // 1) ���� �Ҹ� ���
    if (EcholocationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, EcholocationSound, GetActorLocation());
    }

    // 2) ����׿� �ݰ� �ð�ȭ
   /* DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        PingRadius,
        32,
        FColor::Cyan,
        false,
        RevealDuration
    );*/

    // 3) �ݰ� �� ��� �÷��̾� ĳ���� ǥ��
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
                // ����: ���ü� ���
                Char->SetActorHiddenInGame(false);
                // RevealDuration ���� �ٽ� ������� ���� ���� �ʿ�
            }
        }
    }
}

void ALCBossBanshee::OnHeardNoise(const FVector& NoiseLocation)
{
    // 1) �Ҹ� �߻� ���� �ٶ󺸱�
    FVector Dir = NoiseLocation - GetActorLocation();
    Dir.Z = 0.f;
    if (!Dir.IsNearlyZero())
    {
        SetActorRotation(Dir.Rotation());
    }

    // 2) ���¢��
    if (bCanShriek)
    {
        // ���� ���
        if (SonicShriekSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this, SonicShriekSound, GetActorLocation());
        }

        // �ݰ� ������ ����
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

        // ����� �ð�ȭ
       /* DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            ShriekRadius,
            32,
            FColor::Red,
            false,
            2.f
        );*/

        // ��ٿ� ����
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
