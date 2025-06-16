#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShadowClone.generated.h"

UCLASS()
class LASTCANARY_API AShadowClone : public ACharacter
{
	GENERATED_BODY()

public:
    AShadowClone();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** 분신 생존 시간 (초) */
    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    float LifeTime = 10.f;

    /** 사라질 때 페이드 아웃 시간 (초) */
    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    float FadeOutTime = 1.0f;

    /** 체력이 0 이하가 되면 즉시 사라짐 */
    UPROPERTY(EditAnywhere, Category = "Clone")
    float Health = 1.f;

    /** 타이머 핸들 */
    FTimerHandle LifeTimerHandle;

    /** 페이드 아웃용 머티리얼 인스턴스 */
    UPROPERTY()
    UMaterialInstanceDynamic* DynMaterial;

    /** 원본 메쉬 머티리얼 복사본 */
    UPROPERTY(EditDefaultsOnly, Category = "Clone")
    UMaterialInterface* CloneMaterialBase;

    /** 페이드 아웃 처리 */
    void FadeOutAndDestroy();

    /** 대미지 처리 */
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

};
