#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "DataTable/GunDataRow.h"
#include "Engine/DecalActor.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "GunBase.generated.h"

/**
 * 총기 기본 클래스
 * 라인트레이스 기반 발사 시스템과 탄약 관리를 구현합니다.
 */
UCLASS()
class LASTCANARY_API AGunBase : public AEquipmentItemBase
{
    GENERATED_BODY()

public:
    AGunBase();

    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------

    /** 총구 위치를 나타내는 씬 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Components")
    USceneComponent* MuzzleLocation;

    //-----------------------------------------------------
    // 총기 데이터
    //-----------------------------------------------------

    /** 아이템 데이터 테이블에서의 행 이름은 아이템과 동일하게 설정 */

    /** 아이템 데이터 테이블 참조 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
    UDataTable* GunDataTable;

    /** 아이템의 실제 데이터 (데이터 테이블에서 로드됨) */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Item, meta = (ShowOnlyInnerProperties))
    FGunDataRow GunData;

    //-----------------------------------------------------
    // 총기 속성
    //-----------------------------------------------------

    /** 총의 사거리 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Properties")
    float FireRange;

    /** 총의 데미지 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Properties")
    float BaseDamage;

    /** 발사 간격 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Properties")
    float FireRate;

    /** 발사 정확도 (낮을수록 정확) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Properties")
    float Spread;

    /** 한 번에 발사되는 탄환 수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Properties")
    int32 BulletsPerShot;

    /** 최대 탄약 용량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Properties")
    float MaxAmmo;

    /** 현재 장전된 탄약 (Durability로 대신하게 될 예정) */
    UPROPERTY(BlueprintReadOnly, Category = "Gun|Properties")
    float CurrentAmmo;

    //-----------------------------------------------------
    // 이펙트 및 사운드
    //-----------------------------------------------------

    /** 발사 효과 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    UParticleSystem* MuzzleFlash;

    /** 탄흔 데칼 머티리얼 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    UMaterialInterface* ImpactDecalMaterial;

    /** 데칼 크기 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    FVector DecalSize;

    /** 데칼 지속 시간 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    float DecalLifeSpan;

    /** 발사 사운드 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    USoundBase* FireSound;

    /** 빈 탄창 사운드 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    USoundBase* EmptySound;

    /** 피격 사운드 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    USoundBase* ImpactSound;

    //-----------------------------------------------------
    // 기본 기능
    //-----------------------------------------------------

    /** 아이템 사용 (발사) */
    virtual void UseItem() override;

    /** 탄약 재장전 */
    UFUNCTION(BlueprintCallable, Category = "Gun|Actions")
    virtual bool Reload(float AmmoAmount);

protected:
    virtual void BeginPlay() override;

    /** 마지막 발사 시간 */
    float LastFireTime;

    /** 발사 타이머 핸들 */
    FTimerHandle FireTimerHandle;

    //-----------------------------------------------------
    // 라인트레이스 및 피격 판정
    //-----------------------------------------------------

    /** 라인트레이스 기반 발사 처리 */
    UFUNCTION(Server, Reliable)
    void Server_Fire();
    void Server_Fire_Implementation();

    /** 서버에서 실행되는 발사 로직 */
    void HandleFire();

    /** 라인트레이스 실행 */
    bool PerformLineTrace(FHitResult& OutHit, FVector& StartLocation, FVector& EndLocation);

    /** 피격 처리 */
    void ProcessHit(const FHitResult& HitResult, const FVector& StartLocation);

    /** 피격 지점에 데칼 생성 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SpawnImpactEffects(const FHitResult& Hit);
    void Multicast_SpawnImpactEffects_Implementation(const FHitResult& Hit);

    /** 사운드 및 이펙트 재생 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayFireEffects();
    void Multicast_PlayFireEffects_Implementation();

    //-----------------------------------------------------
    // 상태 관리 및 동기화
    //-----------------------------------------------------
    /** 총기 데이터 테이블 */
    void ApplyGunDataFromDataTable();

    /** 탄약 상태 업데이트 */
    void UpdateAmmoState();

    /** Durability 변경 시 탄약 동기화 */
    virtual void OnRepDurability() override;
};
