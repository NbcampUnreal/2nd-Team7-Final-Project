#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WarderSearch.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_WarderSearch : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WarderSearch();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UFUNCTION()
	void CheckArrival();

	void StopTimer();

	FTimerHandle CheckTimerHandle;
	UBehaviorTreeComponent* CachedOwnerComp = nullptr;
};