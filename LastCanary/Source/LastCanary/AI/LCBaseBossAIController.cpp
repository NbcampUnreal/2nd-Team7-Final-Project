#include "AI/LCBaseBossAIController.h"

ALCBaseBossAIController::ALCBaseBossAIController()
{
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
        TEXT("PerceptionComponent"));

    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(
        TEXT("BehaviorComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(
        TEXT("BlackboardComp"));
}

void ALCBaseBossAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ALCBaseBossAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 1) BT 초기화 및 실행
    InitializeBehavior();

    // 2) 서브클래스별 감지 설정
    ConfigureSenses();

    // 3) 공통 델리게이트 바인딩
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this, &ALCBaseBossAIController::OnTargetPerceptionUpdated);
    }
}

void ALCBaseBossAIController::OnUnPossess()
{
    // 언소유 시 델리게이트 해제
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(
            this, &ALCBaseBossAIController::OnTargetPerceptionUpdated);
    }
    Super::OnUnPossess();
}

void ALCBaseBossAIController::InitializeBehavior()
{
    if (BehaviorTreeAsset && BlackboardComp && BehaviorComp)
    {
        BlackboardComp->InitializeBlackboard(
            *BehaviorTreeAsset->BlackboardAsset);
        BehaviorComp->StartTree(*BehaviorTreeAsset);
    }
}

void ALCBaseBossAIController::OnTargetPerceptionUpdated(
    AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !BlackboardComp)
        return;

    if (!Stimulus.WasSuccessfullySensed())
    {
        BlackboardComp->ClearValue(TEXT("TargetActor"));
        UE_LOG(LogTemp, Log, TEXT("[BossAI] 타겟 상실: %s"), *Actor->GetName());
        return;
    }

    BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
    UE_LOG(LogTemp, Log, TEXT("[BossAI] 감지: %s"), *Actor->GetName());
}