#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameplayTagAssetInterface.h"
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

    // ExecuteRandomPlayerIllusion 에서 포스트프로세스 제거용
    UFUNCTION()
    void RemovePostProcess(UCameraComponent* Cam);

    // Illusion 콜백 타이머 핸들
    FTimerHandle IllusionTimerHandle;

    // 포스트프로세스 제거용 타이머 핸들
    FTimerHandle RemovePPHandle;

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

    // ── 데미지용 콜리전 캡슐 ─────────────────────────────
    /** 데미지 판정용 캡슐 충돌체 */
    UPROPERTY(VisibleAnywhere, Category = "Illusion|Components")
    UCapsuleComponent* DamageCapsule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
    FGameplayTagContainer GameplayTags;

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const;

    // 데미지를 직접 처리하기 위해 TakeDamage 오버라이드
    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        AController* EventInstigator,
        AActor* DamageCauser
    ) override;

    void DestroyIllusion();
    void PickNewMoveTarget();

};
