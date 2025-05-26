#pragma once

#include "CoreMinimal.h"
#include "AI/BaseAIController.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "LCBossHearingAIController.generated.h"

UCLASS()
class LASTCANARY_API ALCBossHearingAIController : public ABaseAIController
{
	GENERATED_BODY()

public:
	ALCBossHearingAIController();

protected:
	virtual void BeginPlay() override;

	/** 청각 감지 설정 객체 **/
	UPROPERTY()
	UAISenseConfig_Hearing* HearingConfig;

	/** AI가 읽고 쓸 Blackboard 컴포넌트 **/
	UPROPERTY(Transient)
	UBlackboardComponent* BlackboardComp;

	/** Behavior Tree 실행 컴포넌트 **/
	UPROPERTY(Transient)
	UBehaviorTreeComponent* BehaviorComp;

	/** 사용할 Behavior Tree 에셋 **/
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

	/** 지각 이벤트 콜백 **/
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};