#include "AI/Service/BTService_RunEQS_ChaseTarget.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

UBTService_RunEQS_ChaseTarget::UBTService_RunEQS_ChaseTarget()
{
    NodeName = TEXT("Run EQS Chase Target");
    Interval = 0.5f;
    bCreateNodeInstance = true;
    ChaseQuery = nullptr;
}

void UBTService_RunEQS_ChaseTarget::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AICon = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!AICon || !ChaseQuery || !BB) return;

    UEnvQueryInstanceBlueprintWrapper* Instance =
        UEnvQueryManager::RunEQSQuery(
            GetWorld(),
            ChaseQuery,
            AICon->GetPawn(),
            EEnvQueryRunMode::SingleResult,
            nullptr);

    if (Instance)
    {
        QueryToBB.Add(Instance, BB);
        Instance->GetOnQueryFinishedEvent().AddDynamic(
            this, &UBTService_RunEQS_ChaseTarget::OnQueryFinished);
    }
}

void UBTService_RunEQS_ChaseTarget::OnQueryFinished(
    UEnvQueryInstanceBlueprintWrapper* QueryInstance,
    EEnvQueryStatus::Type QueryStatus)
{
    if (QueryStatus != EEnvQueryStatus::Success || !QueryInstance)
        return;

    TArray<FVector> Results;
    if (!QueryInstance->GetQueryResultsAsLocations(Results) || Results.Num() == 0)
        return;

    UBlackboardComponent** BBPtr = QueryToBB.Find(QueryInstance);
    if (!BBPtr) return;

    UBlackboardComponent* BB = *BBPtr;
    BB->SetValueAsVector(ChaseLocationKey.SelectedKeyName, Results[0]);

    QueryToBB.Remove(QueryInstance);
}
