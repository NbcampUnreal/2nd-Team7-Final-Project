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

    /** 서브클래스에서 감지 구성만 수행 */
    virtual void ConfigureSenses() PURE_VIRTUAL(ALCBaseBossAIController::ConfigureSenses, );

    /** 지각 이벤트 처리 (공통 로직) */
    UFUNCTION()
    virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    /** Behavior Tree 초기화 헬퍼 */
    void InitializeBehavior();

    /** BehaviorTree / Blackboard 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UBehaviorTreeComponent* BehaviorComp;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    UBlackboardComponent* BlackboardComp;

    /** 사용할 BT 에셋 */
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;
	
};
