#pragma once

#include "CoreMinimal.h"
#include "AI/BaseAIController.h"
#include "Perception/AIPerceptionTypes.h"
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

	/*--------------  에디터 파라미터 --------------*/
	/** 실행할 기본 BT (보스마다 BP에서 바꿔치우면 됨) */
	UPROPERTY(EditDefaultsOnly, Category = "AI") UBehaviorTree* DefaultBT;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Sight") float SightRadius = 2000.f;
	UPROPERTY(EditDefaultsOnly, Category = "AI|Sight") float LoseSightRadius = 2200.f;
	UPROPERTY(EditDefaultsOnly, Category = "AI|Sight") float SightFOV = 70.f;

	/*--------------  런타임 상태 --------------*/
	UPROPERTY(VisibleInstanceOnly, Category = "AI") AActor* LockedTarget = nullptr;

protected:
	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "AI") UBehaviorTreeComponent* BTComp;
	UPROPERTY(VisibleAnywhere, Category = "AI") UBlackboardComponent* BBComp;
	UPROPERTY(VisibleAnywhere, Category = "AI") UAIPerceptionComponent* Perception;

	UAISenseConfig_Sight* SightConfig;

	/* 블랙보드 키 (FName) */
	static const FName BBKey_TargetActor;
	static const FName BBKey_HasLOS;

	/* 라이프사이클 */
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	/* Perception */
	UFUNCTION() void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stim);

	/* 타깃 유효성 재검사 */
	FTimerHandle ValidateTimer;
	void ValidateLockedTarget();
	
};
