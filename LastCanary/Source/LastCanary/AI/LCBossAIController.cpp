#include "AI/LCBossAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

ALCBossAIController::ALCBossAIController()
{
    // 1) PerceptionComponent 세팅 (부모 클래스의 PerceptionComponent를 그대로 사용)
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // 시각 감지 설정
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.f;
    SightConfig->LoseSightRadius = 2300.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
    SightConfig->SetMaxAge(5.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // 청각 감지 설정
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.f;
    HearingConfig->SetMaxAge(5.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // PerceptionComponent에 감각 등록
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALCBossAIController::OnTargetPerceptionUpdated);

    // 2) Behavior Tree / Blackboard 컴포넌트 생성
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ALCBossAIController::BeginPlay()
{
    Super::BeginPlay();

    if (BehaviorTreeAsset)
    {
        // Blackboard 초기화
        BlackboardComp->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);

        // Behavior Tree 시작
        BehaviorComp->StartTree(*BehaviorTreeAsset);
    }
}

void ALCBossAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    // 감각별 ID 획득
    static const FAISenseID SightID = SightConfig->GetSenseID();
    static const FAISenseID HearingID = HearingConfig->GetSenseID();

    if (Stimulus.WasSuccessfullySensed())
    {
        // Blackboard에 타겟 등록
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);

        if (Stimulus.Type == SightID)
        {
            UE_LOG(LogTemp, Log, TEXT("[BossAI] 시각으로 타겟: %s"), *Actor->GetName());
        }
        else if (Stimulus.Type == HearingID)
        {
            UE_LOG(LogTemp, Log, TEXT("[BossAI] 청각으로 타겟: %s"), *Actor->GetName());
        }
    }
    else
    {
        // 감각 상실 시 Blackboard에서 타겟 해제
        BlackboardComp->ClearValue(TEXT("TargetActor"));
        UE_LOG(LogTemp, Log, TEXT("[BossAI] 타겟 해제: %s"), *Actor->GetName());
    }
}