#include "AI/Clue/ScratchMark.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "Engine/World.h"

AScratchMark::AScratchMark()
{
    PrimaryActorTick.bCanEverTick = false;

    // 루트 컴포넌트 없이 DecalComp를 루트로 사용
    DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
    RootComponent = DecalComp;

    // 너무 멀리 떨어지면 보이지 않도록 FadeScreenSize를 작게 설정
    DecalComp->SetFadeScreenSize(0.001f);
}

void AScratchMark::BeginPlay()
{
    Super::BeginPlay();

    // 1) 머티리얼 세팅
    if (ScratchMarkMaterial)
    {
        DecalComp->SetDecalMaterial(ScratchMarkMaterial);
    }

    // 2) 크기 세팅
    DecalComp->DecalSize = DecalSize;
}
