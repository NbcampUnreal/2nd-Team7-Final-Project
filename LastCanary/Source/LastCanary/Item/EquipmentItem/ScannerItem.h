#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/World.h"
#include "ScannerItem.generated.h"

UCLASS()
class LASTCANARY_API AScannerItem : public AEquipmentItemBase
{
    GENERATED_BODY()

public:
    AScannerItem();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void UseItem() override;
    virtual void SetEquipped(bool bNewEquipped) override;

    /** 서버에서 스캔 처리 (사용자 정보 포함) */
    UFUNCTION(Server, Reliable)
    void Server_UseScannerItem(APlayerController* UserPlayerController);
    void Server_UseScannerItem_Implementation(APlayerController* UserPlayerController);

    /** 클라이언트에 스캔 효과 적용 */
    UFUNCTION(Client, Reliable)
    void Client_ApplyScanEffect(bool bStartScan);
    void Client_ApplyScanEffect_Implementation(bool bStartScan);

private:
    /** 현재 스캔 중인 플레이어 컨트롤러 */
    UPROPERTY()
    APlayerController* CurrentScanningPlayer;

    /** 현재 사용자 플레이어 컨트롤러 */
    UPROPERTY()
    APlayerController* CurrentUserPlayer;

protected:
    //-----------------------------------------------------
    // 스캐너 설정
    //-----------------------------------------------------

    /** 머티리얼 파라미터 컬렉션 참조 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner")
    UMaterialParameterCollection* ScanParameterCollection;

    /** 스캔 중인지 여부 (로컬만) */
    UPROPERTY(BlueprintReadOnly, Category = "Scanner")
    bool bIsScanning;

    /** 스캔 지속 시간 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner")
    float ScanDuration;

    /** 사용당 배터리 소모량 (%) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner")
    float BatteryConsumptionPerUse;

    /** 스캔 시작 시 ScanAlpha 목표 값 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner")
    float TargetScanAlpha;

    /** 스캔 종료 시 ScanAlpha 목표 값 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner")
    float DefaultScanAlpha;

    //-----------------------------------------------------
    // 타이머
    //-----------------------------------------------------

    /** 스캔 타이머 */
    FTimerHandle ScanTimerHandle;

    /** 스캔 알파 보간 타이머 */
    FTimerHandle AlphaInterpolationTimerHandle;

    //-----------------------------------------------------
    // 스캔 함수들
    //-----------------------------------------------------

    /** 스캔 시작 */
    UFUNCTION(BlueprintCallable, Category = "Scanner")
    void StartScan();

    /** 스캔 종료 */
    UFUNCTION(BlueprintCallable, Category = "Scanner")
    void StopScan();

    /** 스캔 알파 값 보간 */
    UFUNCTION()
    void InterpolateScanAlpha();

    /** 즉시 스캔 효과를 0으로 리셋 */
    UFUNCTION(BlueprintCallable, Category = "Scanner")
    void ResetScanEffect();

    //-----------------------------------------------------
    // 유틸리티
    //-----------------------------------------------------

    /** 머티리얼 파라미터 컬렉션에서 ScanAlpha 값 설정 (로컬만) */
    void SetScanAlpha(float Alpha);

    /** 머티리얼 파라미터 컬렉션 인스턴스 가져오기 */
    UMaterialParameterCollectionInstance* GetParameterCollectionInstance() const;

private:
    /** 현재 알파 값 (보간용) */
    float CurrentAlpha;

    /** 목표 알파 값 (보간용) */
    float TargetAlpha;

    /** 보간 속도 */
    float InterpolationSpeed;

    /** 캐시된 파라미터 컬렉션 인스턴스 */
    UPROPERTY()
    UMaterialParameterCollectionInstance* CachedCollectionInstance;
};
