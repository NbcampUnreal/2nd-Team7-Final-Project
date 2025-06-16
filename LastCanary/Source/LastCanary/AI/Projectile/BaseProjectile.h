#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class LASTCANARY_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:
    ABaseProjectile();

    /** 투사체 초기화 함수 */
    virtual void InitProjectile(const FVector& Direction, float Speed, float Damage, AController* InstigatorController);

protected:
    virtual void BeginPlay() override;

    /** 디버그용으로 보여줄 스태틱 메쉬 */
    UPROPERTY(VisibleAnywhere, Category = "Projectile|Debug")
    UStaticMeshComponent* DebugMesh;

    /** 충돌체 */
    UPROPERTY(VisibleDefaultsOnly, Category = "Projectile|Collision")
    USphereComponent* CollisionComp;

    /** 비주얼 이펙트 (스폰 또는 충돌 시) */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile|FX")
    UParticleSystem* SpawnFX;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile|FX")
    UParticleSystem* ImpactFX;

    /** 사운드 */
    UPROPERTY(EditDefaultsOnly, Category = "Projectile|Sound")
    USoundBase* SpawnSound;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile|Sound")
    USoundBase* ImpactSound;

    /** 이동 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "Projectile|Movement")
    UProjectileMovementComponent* MovementComp;

    /** 투사체 데미지 */
    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    float ProjectileDamage;

    /** 인스티게이터 컨트롤러 */
    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    AController* InstigatorControllerRef;

    /** 충돌 핸들러 */
    UFUNCTION()
    virtual void OnHit(
        UPrimitiveComponent* HitComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit
    );

    /** 충돌 시 연출 수행 */
    virtual void PlayImpactEffects(const FVector& Location);

};
