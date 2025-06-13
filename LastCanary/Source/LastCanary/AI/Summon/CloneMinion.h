#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NiagaraSystem.h"
#include "CloneMinion.generated.h"

UCLASS()
class LASTCANARY_API ACloneMinion : public ACharacter
{
	GENERATED_BODY()

public:
    ACloneMinion();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    float Lifetime = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    UNiagaraSystem* DisappearEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    float ExplosionDamage = 50.f;

    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    float ExplosionRadius = 400.f;

    UFUNCTION()
    void SelfDestruct();

private:
    FTimerHandle SelfDestructHandle;
};