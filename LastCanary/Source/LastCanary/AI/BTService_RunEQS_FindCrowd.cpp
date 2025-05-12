#include "AI/BTService_RunEQS_FindCrowd.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQueryManager.h"


// 생성자: 노드 이름과 실행 간격 설정
UBTService_RunEQS_FindCrowd::UBTService_RunEQS_FindCrowd()
{
    NodeName = TEXT("Run EQS - Find Crowd");
    Interval = 0.3f;
}

// TickNode: 매 Interval마다 호출
void UBTService_RunEQS_FindCrowd::TickNode(UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    if (!CrowdQuery) return;

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return;

    // 블랙보드와 키명 로컬 복사
    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    const FName KeyName = CrowdLocKey.SelectedKeyName;

    // EQS 요청 생성
    FEnvQueryRequest Request(CrowdQuery, AICon);

    // Execute: SingleResult 모드, 람다 콜백에서 BB에 기록
    Request.Execute(
        EEnvQueryRunMode::SingleResult,
        FQueryFinishedSignature::CreateLambda(
            [BBComp, KeyName](TSharedPtr<FEnvQueryResult> Result)
            {
                if (Result.IsValid() && Result->Items.Num() > 0)
                {
                    const FVector BestLoc = Result->GetItemAsLocation(0);
                    BBComp->SetValueAsVector(KeyName, BestLoc);
                }
            }
        )
    );
}

// EQS 실행 완료 콜백
void UBTService_RunEQS_FindCrowd::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result,
    FBlackboardKeySelector Key,
    UBehaviorTreeComponent* OwnerComp)
{
    // 1) Result 유효 확인
    if (!Result.IsValid())
        return;

    // 2) Items 배열 크기로 유효 아이템 검사
    if (Result->Items.Num() == 0)
        return;

    // 3) 첫 번째 위치를 블랙보드에 기록
    const FVector BestLocation = Result->GetItemAsLocation(0);
    if (UBlackboardComponent* BB = OwnerComp->GetBlackboardComponent())
    {
        BB->SetValueAsVector(Key.SelectedKeyName, BestLocation);
    }
}
