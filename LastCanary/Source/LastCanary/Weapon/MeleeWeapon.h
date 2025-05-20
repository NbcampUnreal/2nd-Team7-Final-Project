#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "MeleeWeapon.generated.h"

UCLASS()
class LASTCANARY_API AMeleeWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	AMeleeWeapon();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UNiagaraSystem* HitEffect;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	USoundBase* HitSound;

	virtual void UpdateWeaponUI() override;

	virtual void Attack() override;

protected:
		
	UFUNCTION()
	virtual void OnEnemyOverlap(
		UPrimitiveComponent* OverlappedComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyindex, 
		bool bFromSweep, 
		const FHitResult& Sweep);
};
