#include "AI/EnvQueryTest_NearbyPlayerDensity.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UEnvQueryTest_NearbyPlayerDensity::UEnvQueryTest_NearbyPlayerDensity()
{
    Cost = EEnvTestCost::Low;
    ValidItemType = UEnvQueryItemType_Actor::StaticClass();
    SetWorkOnFloatValues(true);

    Radius = 600.f;

    // 순수 점수 계산만 → Score, Range(0~1) 설정
    TestPurpose = EEnvTestPurpose::Score;
    FilterType = EEnvTestFilterType::Range;
}

void UEnvQueryTest_NearbyPlayerDensity::RunTest(FEnvQueryInstance& QueryInstance) const
{
    UObject* QueryOwner = QueryInstance.Owner.Get();
    if (!QueryOwner) return;

    // 1) 플레이어 배열
    TArray<AActor*> PlayerActors;
    UGameplayStatics::GetAllActorsOfClass(QueryOwner, ACharacter::StaticClass(), PlayerActors);
    const int32 MaxNearbyPossible = FMath::Max(PlayerActors.Num() - 1, 1);

    // 2) 후보 반복
    for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
    {
        AActor* Candidate = GetItemActor(QueryInstance, It.GetIndex());
        if (!Candidate)
        {
            // 무효 → score=0, filterMin=0, filterMax=1
            It.SetScore(TestPurpose, FilterType, 0.f, 0.f, 1.f);
            continue;
        }

        // 3) 반경 내 플레이어 수 세기
        int32 Nearby = 0;
        const FVector Center = Candidate->GetActorLocation();
        for (AActor* Other : PlayerActors)
        {
            if (Other == Candidate) continue;
            if (FVector::DistSquared(Other->GetActorLocation(), Center)
                <= FMath::Square(Radius))
            {
                ++Nearby;
            }
        }

        // 4) 0~1 정규화
        const float Normalized = float(Nearby) / MaxNearbyPossible;

        // 5) 점수 부여 (score, filterMin, filterMax)
        It.SetScore(TestPurpose, FilterType, Normalized, 0.f, 1.f);
    }
}
