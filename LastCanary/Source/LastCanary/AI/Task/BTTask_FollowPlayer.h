#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FollowPlayer.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_FollowPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_FollowPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    /** 플레이어에 근접했을 때 멈출 반경 */
    UPROPERTY(EditAnywhere, Category = "FollowPlayer")
    float AcceptanceRadius = 700.f;
	
};
