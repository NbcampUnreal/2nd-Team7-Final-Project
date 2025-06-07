#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_Move.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_Move : public UBTTask_MoveTo
{
    GENERATED_BODY()

public:
    UBTTask_Move();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MyAcceptableRadius;

private:
    UPROPERTY(EditAnywhere)
    float SoundTimer = 2.5f;

    float LastSoundTime = 0.0f;
};
