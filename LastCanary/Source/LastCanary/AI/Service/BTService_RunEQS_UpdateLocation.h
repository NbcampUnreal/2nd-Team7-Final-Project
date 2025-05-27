#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_RunEQS.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BTService_RunEQS_UpdateLocation.generated.h"

class UEnvQuery;
class UEnvQueryInstanceBlueprintWrapper;

UCLASS()
class LASTCANARY_API UBTService_RunEQS_UpdateLocation : public UBTService_RunEQS
{
	GENERATED_BODY()

public:
    UBTService_RunEQS_UpdateLocation();

protected:
    // 실행할 EQS Asset
    UPROPERTY(EditAnywhere, Category = "AI|EQS")
    UEnvQuery* TargetLocationQuery;

    // 결과를 쓸 블랙보드 키 (Vector)
    UPROPERTY(EditAnywhere, Category = "AI|Blackboard")
    FBlackboardKeySelector TargetLocationKey;

    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;

    UFUNCTION()
    void OnQueryFinished(
        UEnvQueryInstanceBlueprintWrapper* QueryInstance,
        EEnvQueryStatus::Type QueryStatus);

private:
    TMap<UEnvQueryInstanceBlueprintWrapper*, UBlackboardComponent*> InstanceToBB;
};
