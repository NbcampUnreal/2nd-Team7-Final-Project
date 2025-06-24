#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TempleElite_Search.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_TempleElite_Search : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TempleElite_Search();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
    UFUNCTION()
    void CheckArrival();

    void StopTimer();

    FTimerHandle CheckTimerHandle;
    UBehaviorTreeComponent* CachedOwnerComp = nullptr;
};
