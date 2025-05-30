#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "LCBaseBossAIController.generated.h"

UCLASS()
class LASTCANARY_API ALCBaseBossAIController : public AAIController
{
	GENERATED_BODY()

public:
    ALCBaseBossAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

protected:
    virtual void BeginPlay() override;
    void InitializeBehavior();

    /** 공통 지각 콜백 */
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // AAIController에 이미 선언된 PerceptionComponent를 그대로 사용

    /** Behavior Tree 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UBehaviorTreeComponent* BehaviorComp;

    /** Blackboard 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UBlackboardComponent* BlackboardComp;

    /** 에디터에서 지정할 BT 에셋 */
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;
	
};
