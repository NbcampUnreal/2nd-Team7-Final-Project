#include "AI/BossSpawner.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "BaseBossMonsterCharacter.h"

ABossSpawner::ABossSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
    SpawnArea->SetBoxExtent(FVector(500, 500, 100));
    SpawnArea->SetupAttachment(RootComponent);
}

void ABossSpawner::BeginPlay()
{
    Super::BeginPlay();

    // 한 번만 호출해서 보스 한 마리 스폰
    SpawnRandomBoss();
}

void ABossSpawner::SpawnRandomBoss()
{
    if (BossTypes.Num() == 0) return;

    // 배열에서 랜덤 인덱스 뽑기
    int32 Idx = FMath::RandRange(0, BossTypes.Num() - 1);
    TSubclassOf<ABaseBossMonsterCharacter> BossClass = BossTypes[Idx];
    if (!BossClass) return;

    // 박스 컴포넌트 안에서 랜덤 위치 계산
    FVector Origin = SpawnArea->GetComponentLocation();
    FVector Ext = SpawnArea->GetScaledBoxExtent();
    FVector RandPt = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Ext);

    // 월드에 보스 스폰
    FActorSpawnParameters Params;
    Params.Owner = this;
    GetWorld()->SpawnActor<ABaseBossMonsterCharacter>(
        BossClass,
        RandPt,
        FRotator::ZeroRotator,
        Params
    );
}