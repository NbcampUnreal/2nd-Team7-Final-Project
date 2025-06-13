#include "AI/LCBossHearingAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "AI/LCBossBanshee.h"

// 블록 상단에 태그 정의 (원하는 위치에 추가)
static const FName BossNoiseTag = TEXT("Boss");

ALCBossHearingAIController::ALCBossHearingAIController()
{
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 5000.f;
    HearingConfig->SetMaxAge(5.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(HearingConfig->GetSenseImplementation());
}


void ALCBossHearingAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    FAISenseID HearingSenseID = UAISense_Hearing::StaticClass()
        ->GetDefaultObject<UAISense>()->GetSenseID();

    float ClosestDistSqr = FLT_MAX;
    FVector ClosestLocation = FVector::ZeroVector;
    AActor* ClosestActor = nullptr;

    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        FActorPerceptionBlueprintInfo Info;
        PerceptionComponent->GetActorsPerception(Actor, Info);

        for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
        {
            // HearingSenseID + 성공 감지 + “Boss” 태그 체크
            if (Stimulus.Type == HearingSenseID
                && Stimulus.WasSuccessfullySensed()
                && Stimulus.Tag == BossNoiseTag)
            {
                float DistSqr = FVector::DistSquared(
                    Stimulus.StimulusLocation,
                    GetPawn()->GetActorLocation()
                );

                if (DistSqr < ClosestDistSqr)
                {
                    ClosestDistSqr = DistSqr;
                    ClosestLocation = Stimulus.StimulusLocation;
                    ClosestActor = Actor;
                }
            }
        }
    }

    if (ClosestActor)
    {
        // 보스에게 반향 알리기
        if (ALCBossBanshee* Banshee = Cast<ALCBossBanshee>(GetPawn()))
        {
            Banshee->OnHeardNoise(ClosestLocation);
        }

        // 블랙보드에 TargetActor 갱신
        if (UBlackboardComponent* BBComp = GetBlackboardComponent())
        {
            BBComp->SetValueAsObject(TargetActorKey, ClosestActor);
        }
    }
}
