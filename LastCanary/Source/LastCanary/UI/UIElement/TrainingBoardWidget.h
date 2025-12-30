#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/TrainingType.h"
#include "TrainingBoardWidget.generated.h"

class UTextBlock;
class UProgressBar;

/**
 * 
 */
UCLASS()
class LASTCANARY_API UTrainingBoardWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
    //-----------------------------------------------------
    // UI 컴포넌트 바인딩
    //-----------------------------------------------------

    /** 세션 상태 텍스트 (진행중/대기중) */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatusText;

    /** 훈련 모드 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ModeText;

    /** 경과 시간 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TimeText;

    /** 총 발사 횟수 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TotalShotsText;

    /** 총 명중 횟수 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TotalHitsText;

    /** 명중률 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* AccuracyText;

    /** 명중률 프로그레스 바 */
    UPROPERTY(meta = (BindWidget))
    UProgressBar* AccuracyBar;

    /** 헤드샷 횟수 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* HeadshotText;

    /** 파괴한 더미 수 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DummiesDestroyedText;

    /** 총 데미지 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TotalDamageText;

    /** 평균 DPS 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* AverageDPSText;

    //-----------------------------------------------------
    // 함수
    //-----------------------------------------------------

    virtual void NativeConstruct() override;

public:
    /** 통계 정보 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void UpdateStats(const FTrainingStats& Stats, bool bSessionActive, ETrainingMode Mode);

    /** 세션 시작 UI 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void OnSessionStart(ETrainingMode Mode);

    /** 세션 종료 UI 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void OnSessionEnd(const FTrainingStats& FinalStats);

    /** 대기 상태 UI 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void ShowIdleState();

private:
    /** 모드 이름 가져오기 */
    FString GetModeName(ETrainingMode Mode) const;

    /** 시간 포맷 (초 -> 분:초) */
    FString FormatTime(float TimeInSeconds) const;
};
