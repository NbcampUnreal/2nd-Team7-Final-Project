#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTargetDistance.generated.h"

UCLASS()
class LASTCANARY_API UBTService_UpdateTargetDistance : public UBTService
{
	GENERATED_BODY()

public:
    UBTService_UpdateTargetDistance();

protected:
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds
    ) override;
	
};
