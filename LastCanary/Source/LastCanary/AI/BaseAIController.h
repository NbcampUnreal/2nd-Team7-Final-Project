#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"

// Forward declarations
class ABaseMonsterCharacter;
class UBehaviorTree;
class UBlackboardData;
class UBehaviorTreeComponent;
class UBlackboardComponent;

UCLASS()
class LASTCANARY_API ABaseAIController : public AAIController
{
    GENERATED_BODY()
public:
    ABaseAIController();
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    // 행동 상태 설정 (블루프린트에서 호출 가능)
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetPatrolling();
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetChasing(AActor* Target);
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetAttacking();

protected:
    virtual void BeginPlay() override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
    UBlackboardComponent* BlackboardComponent;

    // AI 동작 제어용 비헤이비어 트리 및 블랙보드
    UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
    UBlackboardData* BlackboardData;

    // 몬스터 캐릭터 참조
    UPROPERTY()
    ABaseMonsterCharacter* MonsterCharacter;

    // 블랙보드 키 이름들 (블루프린트에서 접근 가능)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName TargetActorKeyName = "TargetActor";

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName StateKeyName = "CurrentState";

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName LastKnownLocationKeyName = "LastKnownLocation";
};