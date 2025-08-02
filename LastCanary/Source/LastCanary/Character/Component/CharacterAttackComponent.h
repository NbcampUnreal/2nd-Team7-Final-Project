#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterAttackComponent.generated.h"

class AItemBase;
class UBoxComponent;

UCLASS()
class LASTCANARY_API UCharacterAttackComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()
public:
	UCharacterAttackComponent();

protected:
	virtual void BeginPlay() override;

protected:
	// 무기 공격
	UPROPERTY()
	AItemBase* EquippedWeapon;

public:

	void StartAttack();
	void EndAttack();

	void SetEquippedWeapon(AItemBase* Weapon);


	// 손/발 공격용
	UPROPERTY()
	UBoxComponent* HandHitBox;

	UPROPERTY()
	TSet<AActor*> HitActors;

	UFUNCTION()
	void OnHandHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	float UnarmedDamage = 15.f;

	void SetupHandHitBox();

	void PerformKickAttack();          // 발차기 등 캐릭터 내장 공격
	void PerformWeaponAttack(UPrimitiveComponent* WeaponCollider); // 무기용 공격


	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float KickBoxExtent = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float KnockbackStrength = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float UpwardKnockback = 200.f;

	void ApplyDamageAndEffects(AActor* Target, const FVector& KnockbackDirection);

public:
	// 공격 활성화/비활성화
	void EnableKickHitBox();
	void DisableKickHitBox();

	// 무기 설정
	void SetWeaponHitBox(UPrimitiveComponent* WeaponHitBox);

private:
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	UPROPERTY()
	UBoxComponent* KickHitBox;

	UPROPERTY()
	UPrimitiveComponent* CurrentWeaponHitBox;

	// 콜백
	UFUNCTION()
	void OnKickHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	void HandleHit(ACharacter* TargetCharacter);
};
