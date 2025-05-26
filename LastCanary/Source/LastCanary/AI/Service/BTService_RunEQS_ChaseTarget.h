// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_RunEQS.h"
#include "BTService_RunEQS_ChaseTarget.generated.h"

class UEnvQuery;
class UEnvQueryInstanceBlueprintWrapper;

UCLASS()
class LASTCANARY_API UBTService_RunEQS_ChaseTarget : public UBTService_RunEQS
{
	GENERATED_BODY()

public:
    UBTService_RunEQS_ChaseTarget();

protected:
    /** 실행할 EQS 에셋 */
    UPROPERTY(EditAnywhere, Category = "AI|EQS")
    UEnvQuery* ChaseQuery;

    /** 결과를 저장할 블랙보드 키 */
    UPROPERTY(EditAnywhere, Category = "AI|Blackboard")
    FBlackboardKeySelector ChaseLocationKey;

    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;

    UFUNCTION()
    void OnQueryFinished(
        UEnvQueryInstanceBlueprintWrapper* QueryInstance,
        EEnvQueryStatus::Type QueryStatus);

private:
    /** EQS 인스턴스 → Blackboard 매핑 */
    TMap<UEnvQueryInstanceBlueprintWrapper*, UBlackboardComponent*> QueryToBB;
};
