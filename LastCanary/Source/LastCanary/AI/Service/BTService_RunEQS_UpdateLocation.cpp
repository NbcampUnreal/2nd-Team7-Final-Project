#include "AI/Service/BTService_RunEQS_UpdateLocation.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

UBTService_RunEQS_UpdateLocation::UBTService_RunEQS_UpdateLocation()
{
    NodeName = TEXT("Run EQS Update TargetLocation");
    Interval = 0.5f;
    bCreateNodeInstance = true;
    TargetLocationQuery = nullptr;
}

void UBTService_RunEQS_UpdateLocation::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AICon = OwnerComp.GetAIOwner();
    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    if (!AICon || !TargetLocationQuery || !BBComp) return;

    UEnvQueryInstanceBlueprintWrapper* EQSInstance =
        UEnvQueryManager::RunEQSQuery(
            GetWorld(),
            TargetLocationQuery,
            AICon->GetPawn(),
            EEnvQueryRunMode::SingleResult,
            nullptr);

    if (EQSInstance)
    {
        InstanceToBB.Add(EQSInstance, BBComp);
        EQSInstance->GetOnQueryFinishedEvent().AddDynamic(
            this, &UBTService_RunEQS_UpdateLocation::OnQueryFinished);
    }
}

void UBTService_RunEQS_UpdateLocation::OnQueryFinished(
    UEnvQueryInstanceBlueprintWrapper* QueryInstance,
    EEnvQueryStatus::Type QueryStatus)
{
    if (QueryStatus != EEnvQueryStatus::Success || !QueryInstance)
        return;

    TArray<FVector> Locations;
    if (!QueryInstance->GetQueryResultsAsLocations(Locations) || Locations.Num() == 0)
        return;

    UBlackboardComponent** BBPtr = InstanceToBB.Find(QueryInstance);
    if (!BBPtr) return;

    // 실제 블랙보드에 결과 저장
    (*BBPtr)->SetValueAsVector(TargetLocationKey.SelectedKeyName, Locations[0]);

    InstanceToBB.Remove(QueryInstance);
}