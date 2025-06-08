#include "Actor/LCDronePath.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"

void ALCDronePath::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	// 실제 게임에서도 보여지게 하려면 이 조건문 제거
	const int32 NumPoints = SplinePath->GetNumberOfSplinePoints();
	for (int32 i = 0; i < NumPoints - 1; ++i)
	{
		const FVector Start = SplinePath->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		const FVector End = SplinePath->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

		DrawDebugLine(GetWorld(), Start, End, FColor::Green, true, 9999.f, 0, 5.f);
	}
#endif
}

ALCDronePath::ALCDronePath()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false; // 서버에서만 사용하고 동기화할 필요 없음

	SplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("SplinePath"));
	SetRootComponent(SplinePath);
	SplinePath->SetClosedLoop(false);
	SplinePath->SetMobility(EComponentMobility::Static);
}
