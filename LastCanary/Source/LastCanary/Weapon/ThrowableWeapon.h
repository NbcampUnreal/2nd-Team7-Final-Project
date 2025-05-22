#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "ThrowableWeapon.generated.h"

class AThrowableProjectile;
class USphereComponent;

UCLASS()
class LASTCANARY_API AThrowableWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	AThrowableWeapon();

	UFUNCTION(BlueprintPure, Category = "ThrowableWeapon")
	virtual int32 GetCurrentQuantity() const;
	virtual int32 GetMaxQuantity() const;
	UFUNCTION(BlueprintCallable, Category = "ThrowableWeapon")
	virtual void Throw();
	UFUNCTION(BlueprintCallable, Category = "ThrowableWeapon")
	virtual void Spawn();
	UFUNCTION(BlueprintCallable, Category = "ThrowableWeapon")
	virtual void AddQuantity(int32 AddNumber);
	UFUNCTION(BlueprintCallable, Category = "ThrowableWeapon")
	virtual void ReadyToExplosion();
	virtual void UpdateWeaponUI() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ThrowableWeapon")
	bool bIsInHand;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ThrowableWeapon")
	USphereComponent* LocationCheck;
	UPROPERTY();
	AThrowableProjectile* ThrowableProjectile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThrowableWeapon")
	TSubclassOf<AThrowableProjectile> ThrowableProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThrowableWeapon")
	int32 MaxQuantity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ThrowableWeapon")
	int32 CurrentQuantity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ThrowableWeapon")
	float ThrowForce;



	FTimerHandle ReloadTimerHandle;

};
