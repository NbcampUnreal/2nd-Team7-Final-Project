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

    // �н��� �÷��̾� ������ ���� �����Ƿ� ������ ���� �ݸ����� ����
    GetCapsuleComponent()->SetCapsuleRadius(30.f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("OverlapAllPawn"));
}

void AShadowClone::BeginPlay()
{
    Super::BeginPlay();

    // ��Ƽ���� ���̳��� �ν��Ͻ� ���� (���̵� �ƿ���)
    if (CloneMaterialBase && GetMesh())
    {
        DynMaterial = UMaterialInstanceDynamic::Create(CloneMaterialBase, this);
        GetMesh()->SetMaterial(0, DynMaterial);
    }

    // ���� �ð� �� ���̵� �ƿ�
    GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &AShadowClone::FadeOutAndDestroy, LifeTime, false);
}

void AShadowClone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ������ ���� ��ġ�� ����ٴϵ���
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
    // ���̵� �ƿ� �ִϸ��̼�: ��Ƽ���� �Ķ���� ���ĸ� ���δ�
    if (DynMaterial)
    {
        // ���ĸ� 0���� õõ�� ����
        DynMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.f);
    }

    // �ı� ���� (���̵�ƿ� �ð� ��)
    GetWorldTimerManager().ClearTimer(LifeTimerHandle);
    GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &AShadowClone::K2_DestroyActor, FadeOutTime, false);
}