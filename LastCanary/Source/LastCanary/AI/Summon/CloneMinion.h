#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
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
    float ExplosionDamage = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    float ExplosionRadius = 400.f;

    // 사운드 및 감쇠 설정
    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    USoundBase* ExplosionSound;

    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    USoundAttenuation* ExplosionAttenuation;

    UFUNCTION()
    void SelfDestruct();

    // 클라이언트 전용 이펙트·사운드 재생
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayExplosionEffects();

private:
    FTimerHandle SelfDestructHandle;
};