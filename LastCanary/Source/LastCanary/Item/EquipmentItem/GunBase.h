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

 /** 발사 모드 열거형 */
UENUM(BlueprintType)
enum class EFireMode : uint8
{
    Single      UMETA(DisplayName = "단발"),
    FullAuto    UMETA(DisplayName = "연발")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimationComplete, UAnimMontage*, CompletedMontage);

class UShellEjectionComponent;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Components")
    UShellEjectionComponent* ShellEjectionComponent;

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

public:
    /** 현재 발사 모드 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|FireMode")
    EFireMode CurrentFireMode = EFireMode::FullAuto;

    /** 연발 사격 중인지 여부 */
    UPROPERTY(BlueprintReadOnly, Category = "Gun|FireMode")
    bool bIsAutoFiring;

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
public:
    /** 아이템 사용 (발사) */
    virtual void UseItem() override;

    /** 탄약 재장전 */
    UFUNCTION(BlueprintCallable, Category = "Gun|Actions")
    virtual bool Reload();

protected:
    /** 단발 사격 실행 */
    UFUNCTION(BlueprintCallable, Category = "Gun|Fire")
    void FireSingle();

    /** 연발 사격 실행 */
    UFUNCTION(BlueprintCallable, Category = "Gun|Fire")
    void FireAuto();

    /** 발사 가능 여부 확인 */
    bool CanFire();

public:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void SetEquipped(bool bNewEquipped) override;

    /** 마지막 발사 시간 */
    float LastFireTime;

    /** 발사 타이머 핸들 */
    FTimerHandle FireTimerHandle;

    /** 연발 타이머 핸들 */
    FTimerHandle AutoFireTimerHandle;

    /** 재장전 타이머 핸들 */
    FTimerHandle ReloadTimerHandle;

    /** 발사 모드 전환 */
    UFUNCTION(BlueprintCallable, Category = "Gun|FireMode")
    void ToggleFireMode();

    /** 발사 모드 전환 (서버 RPC) */
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Gun|FireMode")
    void Server_ToggleFireMode();
    void Server_ToggleFireMode_Implementation();

    /** 연발 사격 시작 */
    UFUNCTION(BlueprintCallable, Category = "Gun|FireMode")
    void StartAutoFire();

    /** 연발 사격 중단 */
    UFUNCTION(BlueprintCallable, Category = "Gun|FireMode")
    void StopAutoFire();

    //-----------------------------------------------------
    // 라인트레이스 및 피격 판정
    //-----------------------------------------------------

    UPROPERTY(Replicated)
    TArray<FHitResult> RecentHits;

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
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_SpawnImpactEffects(const TArray<FHitResult>& Hits);
    void Multicast_SpawnImpactEffects_Implementation(const TArray<FHitResult>& Hits);

    /** 총기 발사 사운드 및 이펙트 재생 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayFireEffects();
    void Multicast_PlayFireEffects_Implementation();

    /** 재장전 사운드 및 이펙트 재생 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayReloadSound();
    void Multicast_PlayReloadSound_Implementation();

    /** 총기 반동 효과 적용 */
    UFUNCTION(Client, Unreliable)
    void Client_PlayCameraShake();
    void Client_PlayCameraShake_Implementation();

    //-----------------------------------------------------
    // 애니메이션 관련
    //-----------------------------------------------------

    /** 발사 애니메이션 재생 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayFireAnimation();
    void Multicast_PlayFireAnimation_Implementation();

    /** 재장전 애니메이션 재생 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayReloadAnimation();
    void Multicast_PlayReloadAnimation_Implementation();

    /** 애니메이션 재생 헬퍼 함수 */
    UFUNCTION(BlueprintCallable, Category = "Gun|Animation")
    void PlayGunAnimation(UAnimMontage* AnimMontage, float PlayRate = 1.0f);

    /** 현재 재생 중인 애니메이션 몽타주 */
    UPROPERTY(BlueprintReadOnly, Category = "Gun|Animation")
    UAnimMontage* CurrentPlayingMontage;

    /** 애니메이션 완료 델리게이트 */
    UPROPERTY(BlueprintAssignable, Category = "Gun|Animation")
    FOnAnimationComplete OnAnimationComplete;

    /** 애니메이션 완료 콜백 */
    UFUNCTION()
    void OnAnimMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    //-----------------------------------------------------
    // 상태 관리 및 동기화
    //-----------------------------------------------------
    /** 총기 데이터 테이블 */
    void ApplyGunDataFromDataTable();

    /** 탄약 상태 업데이트 */
    void UpdateAmmoState();

    UFUNCTION()
    void EnsureGunDataLoaded();

    UFUNCTION()
    bool IsGunDataLoaded() const;

    /** Durability 변경 시 탄약 동기화 */
    virtual void OnRepDurability() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Debug")
    bool bDrawDebugLine = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Debug")
    float DebugDrawDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Debug")
    bool bDrawImpactDebug = false;

    //-----------------------------------------------------
    // 부착물 컴포넌트
    //-----------------------------------------------------
public:
    /** 스코프 메시 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Attachments")
    UStaticMeshComponent* ScopeComponent;

    /** 현재 부착된 스코프 메시 */
    UPROPERTY(BlueprintReadOnly, Category = "Gun|Attachments")
    UStaticMesh* CurrentAttachedScope;

    //-----------------------------------------------------
    // 부착물 관련 함수
    //-----------------------------------------------------

    /** 스코프 부착 */
    UFUNCTION(BlueprintCallable, Category = "Gun|Attachments")
    void AttachScope(UStaticMesh* ScopeMesh, FName SocketName = TEXT("Scope_Socket"));

    /** 스코프 제거 */
    UFUNCTION(BlueprintCallable, Category = "Gun|Attachments")
    void DetachScope();

    /** 스코프가 부착되어 있는지 확인 */
    UFUNCTION(BlueprintPure, Category = "Gun|Attachments")
    bool HasScopeAttached() const;

protected:
    /** 데이터 테이블에서 부착물 적용 */
    void ApplyAttachmentsFromDataTable();
};
