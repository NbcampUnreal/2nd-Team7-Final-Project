#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossAttack.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_BossAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_BossAttack(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

protected:
    /** 실행 시 보스에게 공격 요청 */
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
	
};
