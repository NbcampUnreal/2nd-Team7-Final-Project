#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_TeleportToLocation.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_TeleportToLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
    // FObjectInitializer 생성자 대신 기본 생성자 사용
    UBTTask_TeleportToLocation();

protected:
    /** 텔레포트할 위치가 들어 있는 블랙보드 Vector 키 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DestinationKey;

    /** 텔레포트 후 초기화할 RagePercent(0~1) 블랙보드 Float 키 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector RagePercentKey;

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
};