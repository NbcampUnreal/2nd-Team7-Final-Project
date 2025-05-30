#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "NavigationInvokerComponent.h" 
#include "Net/UnrealNetwork.h"
#include "BaseMonsterCharacter.generated.h"

class USphereComponent;

UCLASS()
class LASTCANARY_API ABaseMonsterCharacter : public ACharacter
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = Navigation, meta = (AllowPrivateAccess = "true"))
    UNavigationInvokerComponent* NavInvoker;

public:
    ABaseMonsterCharacter();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
    USphereComponent* AttackCollider;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nav")
    float NavGenerationradius;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nav")
    float NavRemovalradius;

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property")
    int32 MaxHP;

    /*UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* Idle;*/

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* StartAttack;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* AImove;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* AIDeath;
    
    UFUNCTION(BlueprintCallable, Category = "Attack")
    void EnableAttackCollider();

    UFUNCTION(BlueprintCallable, Category = "Attack")
    void DisableAttackCollider();

    /*UFUNCTION(NetMulticast, Reliable)
    void MulticastIdle();*/

    UFUNCTION(NetMulticast, Reliable)
    void MulticastStartAttack();
    void MulticastStartAttack_Implementation();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastAIMove();
    void MulticastAIMove_Implementation();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastAIDeath();
    void MulticastAIDeath_Implementation();

    UFUNCTION(BlueprintCallable, Category = "Monster")
    virtual void PerformAttack();

    UFUNCTION(Server, Reliable)
    void ServerPerformAttack();
    void ServerPerformAttack_Implementation();

    /*UFUNCTION(BlueprintPure, Category = "Monster")
    bool bIsAttacking() const { return bIsAttacking; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    int32 GetCurrentHP() const { return CurrentHP; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    bool IsDead() const { return bIsDead; }*/

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
    float AttackRange = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
    float AttackDuration = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
    float AttackCooldown = 1.f;

    /*UPROPERTY(BlueprintReadOnly, Category = "Monster|Combat")
    bool bIsAttacking;*/

    FTimerHandle AttackTimerHandle;
    FTimerHandle DeathTimerHandle;

    UFUNCTION()
    void OnAttackFinished();

    UFUNCTION()
    void DestroyActor();
    //void OnAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    float LastAttackTime = 0.f;

    UPROPERTY(Replicated)
    bool bIsAttacking;

    UPROPERTY(Replicated)
    int32 CurrentHP;

    UPROPERTY(Replicated)
    bool bIsDead;

public:
    FORCEINLINE class UNavigationInvokerComponent* GetNavInvoker() const { return NavInvoker; }
};