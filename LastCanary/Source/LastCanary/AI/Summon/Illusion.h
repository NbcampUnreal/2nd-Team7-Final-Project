#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Illusion.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

UCLASS()
class LASTCANARY_API AIllusion : public AActor
{
	GENERATED_BODY()
	
public:
    AIllusion();
    virtual void Tick(float DeltaTime) override;

    /** 보스 참조를 세팅할 때 호출됨 */
    void SetBossOwner(AActor* Boss);

protected:
    virtual void BeginPlay() override;

    // ── 주기적 랜덤 플레이어 Illusion 효과 ────────────────
    /** 체크 반경 */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Fear")
    float IllusionRadius = 800.f;

    /** 실행 간격 */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Fear")
    float IllusionInterval = 5.f;

    /** 적용할 포스트프로세스 머티리얼 */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Fear")
    UMaterialInterface* IllusionPostProcessMaterial;

    /** 블렌드 가중치 */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Fear", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IllusionBlendWeight = 0.7f;

    /** 지속 시간 (초) */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Fear")
    float IllusionDuration = 1.5f;

    FTimerHandle IllusionTimerHandle;
    void ExecuteRandomPlayerIllusion();

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
