#pragma once

#include "CoreMinimal.h"
#include "AI/BaseAIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "LCBossAIController.generated.h"

class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

UCLASS()
class LASTCANARY_API ALCBossAIController : public ABaseAIController
{
	GENERATED_BODY()

public:
    ALCBossAIController();

protected:
    virtual void BeginPlay() override;

    /** Behavior Tree 에서 사용할 Blackboard Component */
    UPROPERTY(Transient)
    UBlackboardComponent* BlackboardComp;

    /** Behavior Tree 를 구동할 컴포넌트 */
    UPROPERTY(Transient)
    UBehaviorTreeComponent* BehaviorComp;

    /** 사용할 Behavior Tree 자산 */
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    /** 시각 감지 설정 */
    UPROPERTY()
    UAISenseConfig_Sight* SightConfig;

    /** 청각 감지 설정 */
    UPROPERTY()
    UAISenseConfig_Hearing* HearingConfig;

    /** 지각 이벤트 콜백 */
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
