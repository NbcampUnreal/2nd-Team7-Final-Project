#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/HarvestingTool.h"
#include "DrawDebugHelpers.h" 
#include "Pickaxe.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API APickaxe : public AHarvestingTool
{
	GENERATED_BODY()
	
public:
    APickaxe();

protected:
    virtual void BeginPlay() override;

    // 채집 시작/종료 시 콜리전 활성화/비활성화
    virtual void UseItem() override;

    // 데미지 콜리전 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    class USphereComponent* DamageCollision;

    // 데미지 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DamageAmount = 1.0f;

    // 곡괭이 근접공격 시 데미지 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamageAmount = 10.0f;

    // 콜리전 활성화 지속 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CollisionActiveDuration = 0.3f;

    // 소켓 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FName DamageSocketName = TEXT("DamageSocket");

    // 디버그 옵션 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDamageCollisionDebug = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugSphereDuration = 3.0f;

private:
    // 콜리전 이벤트 처리
    UFUNCTION()
    void OnDamageCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    // 서버에서 데미지 적용
    UFUNCTION(Server, Reliable)
    void Server_ApplyDamage(AActor* TargetActor);
    void Server_ApplyDamage_Implementation(AActor* TargetActor);

    // 콜리전 활성화/비활성화
    void SetDamageCollisionEnabled(bool bEnabled);


    // 콜리전 자동 비활성화 타이머
    FTimerHandle CollisionTimerHandle;

    // 이미 데미지를 받은 액터들 (중복 데미지 방지)
    TSet<AActor*> DamagedActors;

    void DisableCollisionAfterDelay();

    // 디버그 시각화
    void ShowDebugDamageSphere();

public:
    // 콜리전 활성화/비활성화 (곡괭이 근접공격)
    void SetAttackCollisionEnabled(bool bEnabled);
};
