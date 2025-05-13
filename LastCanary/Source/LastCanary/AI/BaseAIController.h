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
    // �ൿ ���� ���� (�������Ʈ���� ȣ�� ����)
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

    // AI ���� ����� �����̺�� Ʈ�� �� ������
    UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Behavior")
    UBlackboardData* BlackboardData;

    // ���� ĳ���� ����
    UPROPERTY()
    ABaseMonsterCharacter* MonsterCharacter;

    // ������ Ű �̸��� (�������Ʈ���� ���� ����)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName TargetActorKeyName = "TargetActor";

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName StateKeyName = "CurrentState";

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Blackboard")
    FName LastKnownLocationKeyName = "LastKnownLocation";
};