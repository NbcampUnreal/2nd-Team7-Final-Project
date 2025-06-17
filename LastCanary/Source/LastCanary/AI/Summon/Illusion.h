#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Illusion.generated.h"

class UStaticMeshComponent;


UCLASS()
class LASTCANARY_API AIllusion : public AActor
{
	GENERATED_BODY()
	
public:
    AIllusion();
    virtual void Tick(float DeltaTime) override;

    /** 보스 참조를 세팅할 때 호출됩니다 */
    void SetBossOwner(AActor* Boss);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Stats")
    float MaxHealth = 20.f;
    UPROPERTY(VisibleAnywhere, Category = "Illusion|Stats")
    float Health;

    /** 이동 속도 보간 계수 */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Movement")
    float MoveInterpSpeed = 2.f;

    /** 보스 주변 이 반경 안에서만 랜덤 목표 뽑기 */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Movement")
    float MoveRadius = 600.f;

    AActor* BossOwner = nullptr;
    FVector MoveTarget;

    UPROPERTY(VisibleAnywhere, Category = "Illusion|Components")
    UStaticMeshComponent* MeshComp;

    UFUNCTION()
    void OnTakeAnyDamage_Handler(
        AActor* DamagedActor,
        float Damage,
        const UDamageType* DamageType,
        AController* InstigatedBy,
        AActor* DamageCauser);

    void DestroyIllusion();
    void PickNewMoveTarget();

};
