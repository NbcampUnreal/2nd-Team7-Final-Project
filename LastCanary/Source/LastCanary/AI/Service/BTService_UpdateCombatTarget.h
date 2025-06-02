#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateCombatTarget.generated.h"


UCLASS()
class LASTCANARY_API UBTService_UpdateCombatTarget : public UBTService
{
	GENERATED_BODY()

public:
    UBTService_UpdateCombatTarget();

    /** 추적할 액터 키 (Object) */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    /** 이동할 위치 키 (Vector) */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetLocationKey;

protected:
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds
    ) override;
	
};
