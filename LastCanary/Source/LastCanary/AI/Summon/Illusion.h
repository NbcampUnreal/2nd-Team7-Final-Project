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

protected:
    virtual void BeginPlay() override;

    /** 환영이 살아있는 시간 (초) */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion")
    float LifeTime = 8.f;

    /** 최대 체력 */
    UPROPERTY(EditDefaultsOnly, Category = "Illusion|Stats")
    float MaxHealth = 20.f;

    /** 현재 체력 */
    UPROPERTY(VisibleAnywhere, Category = "Illusion|Stats")
    float Health;

    /** 환영 파괴 시 호출될 함수 */
    void DestroyIllusion();

    /** 시각용 메쉬 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "Illusion|Components")
    UStaticMeshComponent* MeshComp;

    /** 생존 타이머 핸들 */
    FTimerHandle LifeTimerHandle;

    /** 대미지 이벤트 핸들 */
    UFUNCTION()
    void OnTakeAnyDamage_Handler(
        AActor* DamagedActor,
        float Damage,
        const UDamageType* DamageType,
        AController* InstigatedBy,
        AActor* DamageCauser);

};
