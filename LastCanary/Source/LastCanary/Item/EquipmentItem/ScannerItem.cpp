// ScannerItem.cpp
#include "ScannerItem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "LastCanary.h"

AScannerItem::AScannerItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // 기본값 설정
    bIsScanning = false;
    ScanDuration = 3.0f;
    BatteryConsumptionPerUse = 20.0f;
    TargetScanAlpha = 1.0f;
    DefaultScanAlpha = 0.0f;
    CurrentAlpha = 0.0f;
    TargetAlpha = 0.0f;
    InterpolationSpeed = 2.0f;
    CachedCollectionInstance = nullptr;

    bReplicates = true;
}

void AScannerItem::BeginPlay()
{
    Super::BeginPlay();

    // ⭐ 파라미터 컬렉션 인스턴스 캐시 (모든 클라이언트에서)
    CachedCollectionInstance = GetParameterCollectionInstance();

    if (CachedCollectionInstance)
    {
        SetScanAlpha(DefaultScanAlpha);
        CurrentAlpha = DefaultScanAlpha;
        TargetAlpha = DefaultScanAlpha;
    }
    else
    {
        LOG_Item_WARNING(TEXT("[ScannerItem::BeginPlay] 경고: ScanParameterCollection이 설정되지 않음"));
    }
}

void AScannerItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ScanTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(AlphaInterpolationTimerHandle);
    }

    // 스캔 종료 시 알파 값 리셋
    if (CachedCollectionInstance)
    {
        SetScanAlpha(DefaultScanAlpha);
    }

    Super::EndPlay(EndPlayReason);
}

void AScannerItem::UseItem()
{
    if (!bIsEquipped)
    {
        LOG_Item_WARNING(TEXT("[ScannerItem::UseItem] 장착되지 않은 스캐너"));
        return;
    }

    APlayerController* UserPC = nullptr;

    // 현재 입력을 보낸 플레이어 찾기
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        UserPC = Cast<APlayerController>(OwnerPawn->GetController());
    }

    if (!UserPC)
    {
        UserPC = GetWorld()->GetFirstPlayerController();
        if (UserPC && !UserPC->IsLocalPlayerController())
        {
            // 멀티플레이어에서 로컬 플레이어 찾기
            for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                APlayerController* PC = It->Get();
                if (PC && PC->IsLocalPlayerController())
                {
                    UserPC = PC;
                    break;
                }
            }
        }
    }

    if (!UserPC)
    {
        LOG_Item_WARNING(TEXT("[ScannerItem::UseItem] 사용자 PlayerController를 찾을 수 없음"));
        return;
    }

    Server_UseScannerItem(UserPC);
}

void AScannerItem::Server_UseScannerItem_Implementation(APlayerController* UserPlayerController)
{
    if (!UserPlayerController)
    {
        LOG_Item_WARNING(TEXT("[Server_UseScannerItem] UserPlayerController가 null"));
        return;
    }

    // 배터리 체크
    if (Durability < BatteryConsumptionPerUse)
    {
        LOG_Item_WARNING(TEXT("[Server_UseScannerItem] 배터리 부족"));
        return;
    }

    // 배터리 소모
    float OldDurability = Durability;
    Durability = FMath::Max(0.0f, Durability - BatteryConsumptionPerUse);

    LOG_Item_WARNING(TEXT("[Server_UseScannerItem] 서버에서 배터리 소모: %.1f → %.1f"),
        OldDurability, Durability);

    OnItemStateChanged.Broadcast();

    CurrentUserPlayer = UserPlayerController;

    // 스캔 상태 토글
    if (bIsScanning && CurrentScanningPlayer == UserPlayerController)
    {
        // 같은 플레이어가 스캔 중단
        LOG_Item_WARNING(TEXT("[Server_UseScannerItem] 스캔 중단"));

        bIsScanning = false;
        CurrentScanningPlayer = nullptr;

        // 타이머 정리
        GetWorld()->GetTimerManager().ClearTimer(ScanTimerHandle);

        // 해당 클라이언트에만 스캔 중단 알림
        UserPlayerController->GetPawn()->ForceNetUpdate();
        FTimerHandle DelayHandle;
        GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this, UserPlayerController]()
            {
                if (UserPlayerController && UserPlayerController->IsValidLowLevel())
                {
                    Client_ApplyScanEffect(false);
                }
            }, 0.1f, false);
    }
    else
    {
        // 새로운 스캔 시작
        LOG_Item_WARNING(TEXT("[Server_UseScannerItem] 스캔 시작"));

        bIsScanning = true;
        CurrentScanningPlayer = UserPlayerController;

        // 스캔 지속 시간 타이머
        if (ScanDuration > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(ScanTimerHandle, [this]()
                {
                    if (CurrentScanningPlayer)
                    {
                        LOG_Item_WARNING(TEXT("[Server_UseScannerItem] 자동 스캔 종료"));
                        bIsScanning = false;

                        // 클라이언트에 종료 알림
                        if (CurrentScanningPlayer->IsValidLowLevel())
                        {
                            Client_ApplyScanEffect(false);
                        }
                        CurrentScanningPlayer = nullptr;
                    }
                }, ScanDuration, false);
        }

        // ⭐ 해당 클라이언트에만 스캔 시작 알림
        UserPlayerController->GetPawn()->ForceNetUpdate();
        FTimerHandle DelayHandle;
        GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this, UserPlayerController]()
            {
                if (UserPlayerController && UserPlayerController->IsValidLowLevel())
                {
                    Client_ApplyScanEffect(true);
                }
            }, 0.1f, false);
    }
}

void AScannerItem::Client_ApplyScanEffect_Implementation(bool bStartScan)
{
    if (!CachedCollectionInstance)
    {
        CachedCollectionInstance = GetParameterCollectionInstance();
    }

    if (!CachedCollectionInstance)
    {
        LOG_Item_WARNING(TEXT("[Client_ApplyScanEffect] CachedCollectionInstance가 null"));
        return;
    }

    if (bStartScan)
    {
        // 스캔 시작
        TargetAlpha = TargetScanAlpha;

        GetWorld()->GetTimerManager().SetTimer(AlphaInterpolationTimerHandle,
            this, &AScannerItem::InterpolateScanAlpha, 0.02f, true);
    }
    else
    {
        // 스캔 종료
        TargetAlpha = DefaultScanAlpha;

        LOG_Item_WARNING(TEXT("[Client_ApplyScanEffect] 스캔 효과 종료 - 목표 알파: %.2f"), TargetAlpha);

        GetWorld()->GetTimerManager().SetTimer(AlphaInterpolationTimerHandle,
            this, &AScannerItem::InterpolateScanAlpha, 0.02f, true);
    }
}

void AScannerItem::StartScan()
{
    if (bIsScanning)
    {
        return;
    }

    if (Durability < BatteryConsumptionPerUse)
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[ScannerItem::StartScan] 스캔 시작"));

    // 배터리 소모 (서버에서만 처리)
    if (GetLocalRole() == ROLE_Authority)
    {
        float OldDurability = Durability;
        Durability = FMath::Max(0.0f, Durability - BatteryConsumptionPerUse);

        LOG_Item_WARNING(TEXT("[StartScan] 서버에서 배터리 소모: %.1f → %.1f"),
            OldDurability, Durability);

        OnItemStateChanged.Broadcast();
    }

    bIsScanning = true;

    if (!CachedCollectionInstance)
    {
        CachedCollectionInstance = GetParameterCollectionInstance();
    }

    if (CachedCollectionInstance)
    {
        TargetAlpha = TargetScanAlpha;

        // 알파 보간 타이머 시작
        GetWorld()->GetTimerManager().SetTimer(AlphaInterpolationTimerHandle,
            this, &AScannerItem::InterpolateScanAlpha, 0.02f, true);

        LOG_Item_WARNING(TEXT("[StartScan] 시각적 효과 시작"));
    }

    // 스캔 지속 시간 타이머
    if (ScanDuration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(ScanTimerHandle,
            this, &AScannerItem::StopScan, ScanDuration, false);
    }
}

void AScannerItem::StopScan()
{
    if (!bIsScanning)
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[ScannerItem::StopScan] 스캔 종료"));

    bIsScanning = false;

    if (CachedCollectionInstance)
    {
        TargetAlpha = DefaultScanAlpha;

        if (!AlphaInterpolationTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().SetTimer(AlphaInterpolationTimerHandle,
                this, &AScannerItem::InterpolateScanAlpha, 0.02f, true);
        }
    }

    // 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ScanTimerHandle);
    }
}

void AScannerItem::InterpolateScanAlpha()
{
    if (!CachedCollectionInstance)
    {
        GetWorld()->GetTimerManager().ClearTimer(AlphaInterpolationTimerHandle);
        return;
    }

    float DeltaTime = 0.02f;
    CurrentAlpha = FMath::FInterpTo(CurrentAlpha, TargetAlpha, DeltaTime, InterpolationSpeed);

    // 직접 알파 값 설정
    CachedCollectionInstance->SetScalarParameterValue(TEXT("ScanAlpha"), CurrentAlpha);

    // 목표에 도달하면 타이머 정리
    if (FMath::IsNearlyEqual(CurrentAlpha, TargetAlpha, 0.01f))
    {
        CurrentAlpha = TargetAlpha;
        CachedCollectionInstance->SetScalarParameterValue(TEXT("ScanAlpha"), CurrentAlpha);
        GetWorld()->GetTimerManager().ClearTimer(AlphaInterpolationTimerHandle);

        LOG_Item_WARNING(TEXT("[InterpolateScanAlpha] 알파 보간 완료: %.2f"), CurrentAlpha);
    }
}

void AScannerItem::SetEquipped(bool bNewEquipped)
{
    Super::SetEquipped(bNewEquipped);

    if (!bNewEquipped)
    {
        if (bIsScanning)
        {
            StopScan();
        }

        ResetScanEffect();
        LOG_Item_WARNING(TEXT("[SetEquipped] 스캐너 해제 - 효과 리셋"));
    }
}

void AScannerItem::ResetScanEffect()
{
    if (!CachedCollectionInstance)
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[ResetScanEffect] 로컬 스캔 효과 즉시 리셋"));

    bIsScanning = false;
    SetScanAlpha(0.0f);
    CurrentAlpha = 0.0f;
    TargetAlpha = 0.0f;

    // 모든 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ScanTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(AlphaInterpolationTimerHandle);
    }
}

void AScannerItem::SetScanAlpha(float Alpha)
{
    if (!CachedCollectionInstance)
    {
        return;
    }

    CachedCollectionInstance->SetScalarParameterValue(TEXT("ScanAlpha"), Alpha);
}

UMaterialParameterCollectionInstance* AScannerItem::GetParameterCollectionInstance() const
{
    if (!ScanParameterCollection || !GetWorld())
    {
        return nullptr;
    }

    return GetWorld()->GetParameterCollectionInstance(ScanParameterCollection);
}
