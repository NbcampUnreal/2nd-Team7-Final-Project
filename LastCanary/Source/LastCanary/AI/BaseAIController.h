#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"

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

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetPatrolling();
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetChasing(AActor* Target);
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetAttacking();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
    UBlackboardData* BlackboardData;

    UPROPERTY()
    ABaseMonsterCharacter* MonsterCharacter;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName TargetActorKeyName = "TargetActor";

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName StateKeyName = "CurrentState";

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName LastKnownLocationKeyName = "LastKnownLocation";
};