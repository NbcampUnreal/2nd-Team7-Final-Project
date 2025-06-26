#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FacePlayer.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_FacePlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_FacePlayer();

    /** 매 Tick 호출하도록 설정 */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    /** 회전 속도 (도/초) */
    UPROPERTY(EditAnywhere, Category = "FacePlayer")
    float RotateSpeed = 180.f;

    /** 이 정도 각도 이내면 완료로 간주 (도) */
    UPROPERTY(EditAnywhere, Category = "FacePlayer")
    float AcceptanceAngle = 5.f;
};
