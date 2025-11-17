#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
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

    /** PerceptionComponent 는 AAIController 에 이미 protected 로 선언되어 있으므로 따로 재선언하지 않습니다. */

    /** Behavior Tree 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UBehaviorTreeComponent* BehaviorComp;

    /** Blackboard 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UBlackboardComponent* BlackboardComp;

    /** 에디터에서 지정할 BT 에셋 */
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    /** 지각 갱신 콜백 */
    UFUNCTION()
    virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    /** 블랙보드 키 */
    static const FName TargetActorKey;
	
};
