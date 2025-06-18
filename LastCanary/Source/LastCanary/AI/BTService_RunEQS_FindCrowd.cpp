#include "AI/BTService_RunEQS_FindCrowd.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"


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

    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    const FName KeyName = CrowdLocKey.SelectedKeyName;

    FEnvQueryRequest Request(CrowdQuery, AICon);
    Request.Execute(
        EEnvQueryRunMode::SingleResult,
        FQueryFinishedSignature::CreateLambda(
            [BBComp, KeyName, AICon](TSharedPtr<FEnvQueryResult> Result)
            {
                if (!Result.IsValid() || Result->Items.Num() == 0)
                    return;

                // (1) EQS가 뱉은 원시 위치
                const FVector RawLoc = Result->GetItemAsLocation(0);

                // (2) NavSys과 투영 준비
                UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(AICon->GetWorld());
                FNavLocation ProjectedLoc;
                const FVector ProjectExtent(200.f, 200.f, 200.f);  // 환경에 맞게 조정

                // (3) NavMesh 위 위치로 투영 (세 인자 버전)
                bool bOnNav = NavSys
                    ? NavSys->ProjectPointToNavigation(RawLoc, ProjectedLoc, ProjectExtent)
                    : false;

                // (4) 블랙보드에 기록
                BBComp->SetValueAsVector(
                    KeyName,
                    bOnNav ? ProjectedLoc.Location : RawLoc
                );
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
