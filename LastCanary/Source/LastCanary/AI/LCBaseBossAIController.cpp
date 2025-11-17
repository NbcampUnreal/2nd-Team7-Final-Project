#include "AI/LCBaseBossAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter.h"

const FName ALCBaseBossAIController::TargetActorKey(TEXT("TargetActor"));

ALCBaseBossAIController::ALCBaseBossAIController()
{
    // 1) PerceptionComponent 인스턴스만 생성 (설정은 자식 클래스에서)
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
        TEXT("PerceptionComponent"));
    PerceptionComponent->bAutoActivate = true;

    // 2) BehaviorTree / Blackboard 컴포넌트 생성
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(
        TEXT("BehaviorComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(
        TEXT("BlackboardComp"));

    // 3) BrainComponent 에 BehaviorComp 를 연결
    BrainComponent = BehaviorComp;
}

void ALCBaseBossAIController::BeginPlay()
{
    Super::BeginPlay();

    // 지각 갱신 이벤트 바인딩
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(
        this, &ALCBaseBossAIController::OnPerceptionUpdated);
}

void ALCBaseBossAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    InitializeBehavior();
}

void ALCBaseBossAIController::OnUnPossess()
{
    PerceptionComponent->OnPerceptionUpdated.RemoveDynamic(
        this, &ALCBaseBossAIController::OnPerceptionUpdated);
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

void ALCBaseBossAIController::OnPerceptionUpdated(
    const TArray<AActor*>& UpdatedActors)
{
    // 1) 시야로 보고 있는 액터들 수집
    TArray<AActor*> Visible;
    PerceptionComponent->GetCurrentlyPerceivedActors(
        UAISense_Sight::StaticClass(), Visible);

    // 2) 시야에 누군가 있으면, 가장 가까운 한 명을 TargetActor 에 세팅
    if (Visible.Num() > 0)
    {
        float BestDist2 = FLT_MAX;
        AActor* BestA = nullptr;
        const FVector MyLoc = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;

        for (AActor* A : Visible)
        {
            if (auto* BC = Cast<ABaseCharacter>(A))
            {
                float d2 = FVector::DistSquared(MyLoc, BC->GetActorLocation());
                if (d2 < BestDist2)
                {
                    BestDist2 = d2;
                    BestA = BC;
                }
            }
        }

        if (BestA)
        {
            BlackboardComp->SetValueAsObject(TargetActorKey, BestA);
            return; // 시야에서 찾았으므로 여기서 끝
        }
    }

    // 3) 시야에 아무도 없지만, 이미 블랙보드에 TargetActor가 남아 있으면
    //    그대로 유지(추격 로직이 계속 돌아가게)
    UObject* CurrentTarget = BlackboardComp->GetValueAsObject(TargetActorKey);
    if (CurrentTarget)
    {
        return;
    }

    // 4) 시야도 없고, 블랙보드에도 타겟이 없으면 클리어
    BlackboardComp->ClearValue(TargetActorKey);
}