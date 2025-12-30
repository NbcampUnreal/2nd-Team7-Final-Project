#include "UI/UIElement/TrainingBoardWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "LastCanary.h"

void UTrainingBoardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기 대기 상태 표시
    ShowIdleState();
}

void UTrainingBoardWidget::UpdateStats(const FTrainingStats& Stats, bool bSessionActive, ETrainingMode Mode)
{
    if (!bSessionActive)
    {
        // 훈련이 끝난 뒤에도 전광판을 볼 수 있도록 주석화
        //ShowIdleState();
        return;
    }

    // 세션 상태
    if (StatusText)
    {
        if (bSessionActive)
        {
            StatusText->SetText(FText::FromString(TEXT("훈련 진행 중")));
        }
        else
        {
            StatusText->SetText(FText::FromString(TEXT("훈련 종료 - 결과 확인")));
        }
    }

    // 훈련 모드
    if (ModeText)
    {
        ModeText->SetText(FText::FromString(GetModeName(Mode)));
    }

    // 경과 시간
    if (TimeText)
    {
        TimeText->SetText(FText::FromString(FormatTime(Stats.SessionTime)));
    }

    // 총 발사 횟수
    if (TotalShotsText)
    {
        TotalShotsText->SetText(FText::AsNumber(Stats.TotalShots));
    }

    // 총 명중 횟수
    if (TotalHitsText)
    {
        TotalHitsText->SetText(FText::AsNumber(Stats.TotalHits));
    }

    // 명중률
    if (AccuracyText)
    {
        FString AccuracyStr = FString::Printf(TEXT("%.1f%%"), Stats.AccuracyPercentage);
        AccuracyText->SetText(FText::FromString(AccuracyStr));
    }

    if (AccuracyBar)
    {
        AccuracyBar->SetPercent(Stats.AccuracyPercentage / 100.0f);
    }

    // 헤드샷
    if (HeadshotText)
    {
        HeadshotText->SetText(FText::AsNumber(Stats.HeadshotCount));
    }

    // 파괴한 더미
    if (DummiesDestroyedText)
    {
        DummiesDestroyedText->SetText(FText::AsNumber(Stats.DummiesDestroyed));
    }

    // 총 데미지
    if (TotalDamageText)
    {
        FString DamageStr = FString::Printf(TEXT("%.0f"), Stats.TotalDamageDealt);
        TotalDamageText->SetText(FText::FromString(DamageStr));
    }

    // 평균 DPS
    if (AverageDPSText)
    {
        FString DPSStr = FString::Printf(TEXT("%.1f"), Stats.AverageDPS);
        AverageDPSText->SetText(FText::FromString(DPSStr));
    }
}

void UTrainingBoardWidget::OnSessionStart(ETrainingMode Mode)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("훈련 시작!")));
    }

    if (ModeText)
    {
        ModeText->SetText(FText::FromString(GetModeName(Mode)));
    }

    LOG_Item_WARNING(TEXT("[TrainingBoardWidget] 세션 시작 - 모드: %s"), *GetModeName(Mode));
}

void UTrainingBoardWidget::OnSessionEnd(const FTrainingStats& FinalStats)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("훈련 종료")));
    }

    // 최종 통계 표시
    UpdateStats(FinalStats, false, ETrainingMode::Practice);

    LOG_Item_WARNING(TEXT("[TrainingBoardWidget] 세션 종료 - 최종 명중률: %.1f%%"), FinalStats.AccuracyPercentage);
}

void UTrainingBoardWidget::ShowIdleState()
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("대기 중")));
    }

    if (ModeText)
    {
        ModeText->SetText(FText::FromString(TEXT("-")));
    }

    if (TimeText)
    {
        TimeText->SetText(FText::FromString(TEXT("00:00")));
    }

    if (TotalShotsText)
    {
        TotalShotsText->SetText(FText::FromString(TEXT("0")));
    }

    if (TotalHitsText)
    {
        TotalHitsText->SetText(FText::FromString(TEXT("0")));
    }

    if (AccuracyText)
    {
        AccuracyText->SetText(FText::FromString(TEXT("0.0%")));
    }

    if (AccuracyBar)
    {
        AccuracyBar->SetPercent(0.0f);
    }

    if (HeadshotText)
    {
        HeadshotText->SetText(FText::FromString(TEXT("0")));
    }

    if (DummiesDestroyedText)
    {
        DummiesDestroyedText->SetText(FText::FromString(TEXT("0")));
    }

    if (TotalDamageText)
    {
        TotalDamageText->SetText(FText::FromString(TEXT("0")));
    }

    if (AverageDPSText)
    {
        AverageDPSText->SetText(FText::FromString(TEXT("0.0")));
    }
}

FString UTrainingBoardWidget::GetModeName(ETrainingMode Mode) const
{
    switch (Mode)
    {
    case ETrainingMode::Practice:
        return TEXT("연습");
    case ETrainingMode::Timed:
        return TEXT("시간제");
    case ETrainingMode::Survival:
        return TEXT("서바이벌");
    default:
        return TEXT("알 수 없음");
    }
}

FString UTrainingBoardWidget::FormatTime(float TimeInSeconds) const
{
    int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
    int32 Seconds = FMath::FloorToInt(TimeInSeconds) % 60;

    return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}