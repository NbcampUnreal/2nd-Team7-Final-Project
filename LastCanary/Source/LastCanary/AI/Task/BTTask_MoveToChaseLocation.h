#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToChaseLocation.generated.h"


UCLASS()
class LASTCANARY_API UBTTask_MoveToChaseLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_MoveToChaseLocation(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

    /** 도착 인정 반경 */
    UPROPERTY(EditAnywhere, Category = "AI|Movement")
    float AcceptanceRadius;

protected:
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
	
};
