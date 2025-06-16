#include "AI/Summon/Illusion.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

AIllusion::AIllusion()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    // Mesh 초기화
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    MeshComp->SetCastShadow(false);

    // 대미지 이벤트 바인딩
    OnTakeAnyDamage.AddDynamic(this, &AIllusion::OnTakeAnyDamage_Handler);
}

void AIllusion::BeginPlay()
{
    Super::BeginPlay();

    // 체력 초기화
    Health = MaxHealth;

    // LifeTime 경과 시 제거
    if (LifeTime > 0.f)
    {
        GetWorldTimerManager().SetTimer(
            LifeTimerHandle,
            this, &AIllusion::DestroyIllusion,
            LifeTime, false
        );
    }
}

void AIllusion::OnTakeAnyDamage_Handler(
    AActor* DamagedActor,
    float Damage,
    const UDamageType* DamageType,
    AController* InstigatedBy,
    AActor* DamageCauser)
{
    Health -= Damage;
    if (Health <= 0.f)
    {
        DestroyIllusion();
    }
}

void AIllusion::DestroyIllusion()
{
    Destroy();
}
