#include "AI/LCBossHearingAIController.h"
#include "Perception/AIPerceptionComponent.h"

ALCBossHearingAIController::ALCBossHearingAIController()
{
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(
        TEXT("HearingConfig"));
    // 감지 반경을 크게 잡아 멀리서도 소리를 들을 수 있게 합니다.
    HearingConfig->HearingRange = 5000.f;

    // 자극 보관 시간을 늘려, 잠깐 들린 소리라도 일정 시간 동안 유지되도록 합니다.
    HearingConfig->SetMaxAge(10.f);

    // 적·중립·아군 모두 감지
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
}

void ALCBossHearingAIController::ConfigureSenses()
{
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(
        HearingConfig->GetSenseImplementation());
}