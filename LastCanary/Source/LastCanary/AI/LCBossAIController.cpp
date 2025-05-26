#include "AI/LCBossAIController.h"
#include "Perception/AIPerceptionComponent.h"

ALCBossAIController::ALCBossAIController()
{
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(
        TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.f;                          // 시야 반경
    SightConfig->LoseSightRadius = 2300.f;                      // 시야 놓치는 반경
    SightConfig->PeripheralVisionAngleDegrees = 90.f;           // 말초 시야 각도
    SightConfig->SetMaxAge(5.f);                                // 자극 보관 시간(초)
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;  // 적 감지
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;  // 중립 감지
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;  // 아군 감지

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(
        TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.f;                      // 청취 반경
    HearingConfig->SetMaxAge(5.f);                             // 자극 보관 시간(초)
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
}

void ALCBossAIController::ConfigureSenses()
{
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(
        SightConfig->GetSenseImplementation());
}
