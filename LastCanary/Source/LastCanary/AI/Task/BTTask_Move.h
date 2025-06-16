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

protected:
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MyAcceptableRadius;

    /** 이동 체크 타이머 핸들 */
    FTimerHandle MoveCheckTimer;

    /** 타이머로 이동 상태 체크 */
    UFUNCTION()
    void CheckMoveStatus();

    /** 현재 BehaviorTreeComponent 참조 저장 */
    UPROPERTY()
    UBehaviorTreeComponent* CurrentOwnerComp;

private:
    UPROPERTY(EditAnywhere)
    float SoundTimer = 2.5f;

    float LastSoundTime = 0.0f;
};
