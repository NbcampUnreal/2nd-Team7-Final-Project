#include "AI/Projectile/BaseProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseProjectile::ABaseProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    // 충돌 컴포넌트
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->InitSphereRadius(15.f);
    CollisionComp->SetCollisionProfileName("BlockAllDynamic");
    CollisionComp->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
    RootComponent = CollisionComp;

    // 이동 컴포넌트
    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComp"));
    MovementComp->UpdatedComponent = CollisionComp;
    MovementComp->InitialSpeed = 1000.f;
    MovementComp->MaxSpeed = 1000.f;
    MovementComp->bRotationFollowsVelocity = true;
    MovementComp->bShouldBounce = false;

    ProjectileDamage = 10.f;
    InstigatorControllerRef = nullptr;

    // 중력을 안받게함
    MovementComp->ProjectileGravityScale = 0.f;

    // 디버그용 메쉬 세팅
    DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
    DebugMesh->SetupAttachment(RootComponent);
    DebugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DebugMesh->SetRelativeScale3D(FVector(0.2f));  // 작게
}

void ABaseProjectile::BeginPlay()
{
    Super::BeginPlay();

    // 스폰 연출
    if (SpawnFX)
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SpawnFX, GetActorLocation());
    if (SpawnSound)
        UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
}

void ABaseProjectile::InitProjectile(const FVector& Direction, float Speed, float Damage, AController* InstigatorController)
{
    if (MovementComp)
        MovementComp->Velocity = Direction.GetSafeNormal() * Speed;

    ProjectileDamage = Damage;
    InstigatorControllerRef = InstigatorController;
    SetInstigator(InstigatorController ? InstigatorController->GetPawn() : nullptr);
}

void ABaseProjectile::OnHit(
    UPrimitiveComponent* HitComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    if (OtherActor && OtherActor != this && OtherComp)
    {
        UGameplayStatics::ApplyDamage(
            OtherActor,
            ProjectileDamage,
            InstigatorControllerRef,
            this,
            nullptr
        );

        PlayImpactEffects(Hit.ImpactPoint);
    }

    Destroy();
}

void ABaseProjectile::PlayImpactEffects(const FVector& Location)
{
    if (ImpactFX)
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, Location);
    if (ImpactSound)
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Location);
}