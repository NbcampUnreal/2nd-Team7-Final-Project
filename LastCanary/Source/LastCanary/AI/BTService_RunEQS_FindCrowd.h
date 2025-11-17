#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/Services/BTService_RunEQS.h"
#include "BTService_RunEQS_FindCrowd.generated.h"

class UEnvQuery;
class UBehaviorTreeComponent;
struct FEnvQueryResult;

UCLASS()
class LASTCANARY_API UBTService_RunEQS_FindCrowd : public UBTService_RunEQS
{
	GENERATED_BODY()

public:
    UBTService_RunEQS_FindCrowd();

protected:
    /** 사용할 EQS Query 에셋 */
    UPROPERTY(EditDefaultsOnly, Category = "EQS")
    UEnvQuery* CrowdQuery;

    /** 결과를 저장할 블랙보드 Vector 키 (CrowdLoc) */
    UPROPERTY(EditDefaultsOnly, Category = "EQS")
    FBlackboardKeySelector CrowdLocKey;

    /** 0.3초 간격으로 호출되어 EQS를 실행 */
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;

private:
    /** EQS 실행 완료 시 호출되는 콜백 */
    void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result,
        FBlackboardKeySelector Key,
        UBehaviorTreeComponent* OwnerComp);
	
};
