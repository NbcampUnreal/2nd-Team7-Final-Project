#include "AI/LCBaseBossAIController.h"

ALCBaseBossAIController::ALCBaseBossAIController()
{
    // PerceptionComponent는 AAIController가 이미 소유하므로 Create 없이 바로 사용
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
        TEXT("PerceptionComponent"));
    PerceptionComponent->bAutoActivate = true;

    // Behavior Tree / Blackboard 컴포넌트 생성
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

    // BT 실행을 위해 BrainComponent에 할당
    BrainComponent = BehaviorComp;
}

void ALCBaseBossAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ALCBaseBossAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    UE_LOG(LogTemp, Warning, TEXT(">>> OnPossess 호출: %s"), *InPawn->GetName());
    InitializeBehavior();
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
        this, &ALCBaseBossAIController::OnTargetPerceptionUpdated);
}

void ALCBaseBossAIController::OnUnPossess()
{
    PerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(
        this, &ALCBaseBossAIController::OnTargetPerceptionUpdated);
    Super::OnUnPossess();
}

void ALCBaseBossAIController::InitializeBehavior()
{
    if (BehaviorTreeAsset && BlackboardComp && BehaviorComp)
    {
        BlackboardComp->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);
        BehaviorComp->StartTree(*BehaviorTreeAsset);
    }
}

void ALCBaseBossAIController::OnTargetPerceptionUpdated(
    AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !BlackboardComp) return;

    if (!Stimulus.WasSuccessfullySensed())
    {
        BlackboardComp->ClearValue(TEXT("TargetActor"));
        UE_LOG(LogTemp, Log, TEXT("[BossAI] 타겟 상실: %s"), *Actor->GetName());
    }
    else
    {
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
        UE_LOG(LogTemp, Log, TEXT("[BossAI] 감지: %s"), *Actor->GetName());
    }
}