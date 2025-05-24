#include "Framework/Manager/ResultEvaluator.h"

#include "DataTable/ResourceItemRow.h"
#include "DataTable/ResourceCategoryRow.h"
#include "DataType/GameResultData.h"
#include "DataTable/RankThresholdRow.h"

#include "LastCanary.h"

FGameResultData UResultEvaluator::EvaluateResult(
    const TArray<FChecklistQuestion>& PlayerAnswers,
    const TArray<bool>& CorrectAnswers,
    int32 SurvivingPlayers,
    const TMap<FName, int32>& CollectedResources)
{
    const FString Context = TEXT("EvaluateResult");

    // 1. 체크리스트 정답 수 계산
    int32 CorrectCount = 0;
    for (int32 i = 0; i < FMath::Min(PlayerAnswers.Num(), CorrectAnswers.Num()); ++i)
    {
        if (PlayerAnswers[i].bIsAnswered && PlayerAnswers[i].bAnswer == CorrectAnswers[i])
        {
            ++CorrectCount;
        }
    }

    const int32 Total = CorrectAnswers.Num();
    int32 Score = 0;

    // 2. 정답 점수
    Score += CorrectCount * 100;

    // 3. 생존자 점수
    Score += SurvivingPlayers * 50;

    // 4. 자원 점수 (혼합형 계산)
    int32 ResourceScoreSum = 0;
    for (const auto& Pair : CollectedResources)
    {
        const FName& ResourceID = Pair.Key;
        int32 Amount = Pair.Value;

        const FResourceItemRow* ItemRow = ResourceItemTable
            ? ResourceItemTable->FindRow<FResourceItemRow>(ResourceID, Context)
            : nullptr;

        if (ItemRow == nullptr)
        {
            LOG_Frame_WARNING(TEXT("자원 정보 누락: %s"), *ResourceID.ToString());
            continue;
        }

        // 카테고리 → 문자열로 Enum명 추출 후 조회
        FString CategoryRowKey = UEnum::GetValueAsString(ItemRow->Category).RightChop(FString("EResourceCategory::").Len());

        const FResourceCategoryRow* CategoryRow = ResourceCategoryTable
            ? ResourceCategoryTable->FindRow<FResourceCategoryRow>(*CategoryRowKey, Context)
            : nullptr;

        float Multiplier = CategoryRow ? CategoryRow->ScoreMultiplier : 1.0f;
        int32 FinalItemScore = FMath::RoundToInt(ItemRow->BaseScore * Multiplier) * Amount;

        ResourceScoreSum += FinalItemScore;

        LOG_Frame_WARNING(TEXT("[EvaluateResult] %s × %d → %d × %.2f × %d = %d"),
            *ResourceID.ToString(),
            Amount,
            ItemRow->BaseScore,
            Multiplier,
            Amount,
            FinalItemScore
        );
    }

    Score += ResourceScoreSum;

    // 5. 랭크 산정 (임시 하드코딩, 추후 DataTable로 분리 예정)
    FString Rank = "C"; // Default fallback

    if (RankThresholdTable)
    {
        TArray<FRankThresholdRow*> RankRows;
        RankThresholdTable->GetAllRows(TEXT("Rank Lookup"), RankRows);

        // 점수 높은 순서로 정렬
        RankRows.Sort([](const FRankThresholdRow& A, const FRankThresholdRow& B) {
            return A.MinScore > B.MinScore;
            });

        for (const auto* Row : RankRows)
        {
            if (Score >= Row->MinScore)
            {
                Rank = Row->Rank;
                break;
            }
        }

        LOG_Frame_WARNING(TEXT("랭크 판정 기준: 총점 %d → Rank %s"), Score, *Rank);
    }
    else
    {
        LOG_Frame_WARNING(TEXT("RankThresholdTable이 설정되지 않았습니다. 기본 랭크 C로 설정됩니다."));
    }


    // 6. 결과 패킹
    FGameResultData Result;
    Result.CorrectChecklistCount = CorrectCount;
    Result.TotalChecklistCount = Total;
    Result.SurvivingPlayerCount = SurvivingPlayers;
    Result.CollectedResourcePoints = ResourceScoreSum;
    Result.FinalScore = Score;
    Result.Rank = Rank;

    LOG_Frame_WARNING(TEXT("EvaluateResult 완료 - 정답:%d/%d 생존:%d 자원점수:%d → 총점:%d 랭크:%s"),
        CorrectCount, 
        Total,
        SurvivingPlayers,
        ResourceScoreSum, 
        Score,
        *Rank);

    return Result;
}
