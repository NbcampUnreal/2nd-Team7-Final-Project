#include "AI/Summon/FoxfireActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Character/BaseCharacter.h"

AFoxfireActor::AFoxfireActor()
{
    PrimaryActorTick.bCanEverTick = false;

    DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
    DamageSphere->InitSphereRadius(100.f);
    DamageSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DamageSphere->SetupAttachment(RootComponent);
    DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &AFoxfireActor::OnOverlapBegin);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(DamageSphere);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFoxfireActor::BeginPlay()
{
    Super::BeginPlay();

    SetLifeSpan(10.f);
}

void AFoxfireActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // 플레이어 캐릭터에게만 반응
    if (Cast<ABaseCharacter>(OtherActor))
    {
        // 데미지 적용
        UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, nullptr, this, nullptr);

        // 접촉 즉시 파괴
        Destroy();
    }
}