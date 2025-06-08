#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "NavigationInvokerComponent.h" 
#include "Net/UnrealNetwork.h"
#include "GameplayTagAssetInterface.h"
#include "BaseMonsterCharacter.generated.h"

class USphereComponent;
class ABaseCharacter;

UCLASS()
class LASTCANARY_API ABaseMonsterCharacter : public ACharacter, public IGameplayTagAssetInterface
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

    //공격
    UFUNCTION()
    void OnAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    //사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AttackSound1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AttackSound2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AttackSound3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* DeathSound1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* DeathSound2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* MoveSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ChaseSound1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ChaseSound2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ChaseSound3;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAttackSound1();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAttackSound2();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAttackSound3();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDeathSound1();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDeathSound2();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayIdleSound();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayMoveSound();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayChaseSound();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayChaseSound1();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayChaseSound2();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayChaseSound3();

    //FORCEINLINE 
    float GetAttackRange() const { return AttackRange; }

protected:
    UFUNCTION()
    void StopAllCurrentActions();

    //멀티 사운드 재생
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlaySound(USoundBase* Sound);
    void MulticastPlaySound_Implementation(USoundBase* Sound);

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
    int32 MaxHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Combat")
    float AttackRange = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
    float AttackDuration = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
    float AttackCooldown = 1.f;

    FTimerHandle AttackTimerHandle;
    FTimerHandle DeathTimerHandle;
    FTimerHandle AttackEnableTimerHandle;

    UFUNCTION()
    void OnAttackFinished();

    UFUNCTION()
    void DestroyActor();

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
    FGameplayTagContainer GameplayTags;

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

    /** 스캐너를 위한 스텐실 설정 */
    void EnableStencilForAllMeshes(int32 StencilValue);
};