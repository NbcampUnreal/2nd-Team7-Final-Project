#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CallCaveMonster.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_CallCaveMonster : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CallCaveMonster();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UFUNCTION()
	void CheckArrival();

	void StopTimer();

	FTimerHandle CheckTimerHandle;
	UBehaviorTreeComponent* CachedOwnerComp = nullptr;
};