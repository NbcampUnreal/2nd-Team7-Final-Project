#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowableProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class LASTCANARY_API AThrowableProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AThrowableProjectile();

	UFUNCTION(BlueprintCallable, Category = "ThrowableWeapon")
	virtual void ExplosionCount();
	virtual void Thrown(float ThrowForce);
	virtual void SpawnSetting();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USkeletalMeshComponent* ThrowableWeaponMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	USphereComponent* DamageCollision;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UNiagaraSystem* ExplosionNiagara;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	USoundBase* ExplosionSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	USoundBase* ReadyToAttackSound;
	UPROPERTY(VisibleAnywhere, Category = "Component")
	UProjectileMovementComponent* ProjectileMovement;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float DamageRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float TimeForExplosion;

	FTimerHandle WeaponTimerHandle;

	UFUNCTION()
	virtual void Explode();
};
