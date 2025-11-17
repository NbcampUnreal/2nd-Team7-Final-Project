#include "Framework/Manager/ResultEvaluator.h"

#include "DataTable/ItemDataRow.h"
#include "DataTable/ResourceCategoryRow.h"
#include "DataType/GameResultData.h"
#include "DataTable/RankThresholdRow.h"

#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

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

    TArray<FResourceScoreInfo> ResourceScoreDetails;
    LOG_Frame_WARNING(TEXT("CollectedResources.Num(): %d"), CollectedResources.Num());

    for (const auto& Pair : CollectedResources)
    {
        const FName& ResourceID = Pair.Key;
        int32 Amount = Pair.Value;
        LOG_Frame_WARNING(TEXT("Checking resource: %s × %d"), *ResourceID.ToString(), Amount);

        const FItemDataRow* ItemRow = ResourceItemTable
            ? ResourceItemTable->FindRow<FItemDataRow>(ResourceID, Context)
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

        FResourceScoreInfo ResourceScoreInfo;
        ResourceScoreInfo.ResourceID = ResourceID;
        ResourceScoreInfo.BaseScore = ItemRow->BaseScore;
        ResourceScoreInfo.Multiplier = Multiplier;
        ResourceScoreInfo.Amount = Amount;
        ResourceScoreInfo.TotalScore = FinalItemScore;
        ResourceScoreDetails.Add(ResourceScoreInfo);
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
    Result.ResourceScoreDetails = ResourceScoreDetails;
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


FGameResultData UResultEvaluator::EvaluatePlayerResult
(
    const TArray<FChecklistQuestion>& PlayerAnswers,
    const TArray<bool>& CorrectAnswers,
    const bool bIsSurvive,
    const int32 SurviveTime,
    const int32 KillCount,
    const TMap<FName, int32>& CollectedResources,
    const TArray<int32> CollectedClues
)
{
    ULCGameInstanceSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
    if (!Subsystem)
    {
        LOG_Game_ERROR(TEXT("LC GameInstance Subsystem Casting Fail!!"));
        return FGameResultData();
    }

    const FString Context = TEXT("EvaluateResult");

    int32 ResourceScoreSum = 0;
    int32 TotalExplorePoint = 0;

    // 1. 체크리스트 정답 수 계산
    int32 CorrectCount = 0;
    for (int32 i = 0; i < FMath::Min(PlayerAnswers.Num(), CorrectAnswers.Num()); ++i)
    {
        if (PlayerAnswers[i].bIsAnswered && PlayerAnswers[i].bAnswer == CorrectAnswers[i])
        {
            ++CorrectCount;
        }
    }

    const int32 TotalChecklistCount = CorrectAnswers.Num();
   
    //int32 BonusPoint = 0;

    // 2. 정답 점수
    float CorrectRate = (float)CorrectCount / TotalChecklistCount;
    int32 RoundedPoint = FMath::RoundToInt(CorrectRate * 100);

    TotalExplorePoint += CorrectRate;

    // 3. 생존자 여부에 따른 점수
    if (bIsSurvive)
    {
        TotalExplorePoint += 50;
    }

    // 생존시간 에 따른 탐사포인트
    TotalExplorePoint += SurviveTime * 0.5f;

    // 처치 수에 따른 탐사포인트
    TotalExplorePoint += KillCount * 20;

    // 4. 자원 점수 (혼합형 계산)

    TArray<FResourceScoreInfo> ResourceScoreDetails;
    LOG_Frame_WARNING(TEXT("CollectedResources.Num(): %d"), CollectedResources.Num());

    for (const auto& Pair : CollectedResources)
    {
        const FName& ResourceID = Pair.Key;
        int32 Amount = Pair.Value;
        LOG_Frame_WARNING(TEXT("Checking resource: %s × %d"), *ResourceID.ToString(), Amount);

        const FItemDataRow* ItemRow = ResourceItemTable
            ? ResourceItemTable->FindRow<FItemDataRow>(ResourceID, Context)
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

        FResourceScoreInfo ResourceScoreInfo;
        ResourceScoreInfo.ResourceID = ResourceID;
        ResourceScoreInfo.BaseScore = ItemRow->BaseScore;
        ResourceScoreInfo.Multiplier = Multiplier;
        ResourceScoreInfo.Amount = Amount;
        ResourceScoreInfo.TotalScore = FinalItemScore;
        ResourceScoreDetails.Add(ResourceScoreInfo);
    }

    //BonusPoint += ResourceScoreSum;

    // 5. 탐사 아이템 포인트 산정
    TArray<FExplorePointInfo> EXPDetails;

    for (const auto& ClueItemID : CollectedClues)
    {
        FItemDataRow* ClueItem = Subsystem->GetItemDataByItemID(ClueItemID);
        if (ClueItem->bIsNoteItem)
        {
            FExplorePointInfo EXPInfo;
            EXPInfo.NoteTypeIndex = static_cast<int32>(ClueItem->NoteType);
            EXPInfo.ExplorePoint = ClueItem->BaseExplorePoint;

            // TO DO : 아이템 의 BaseExplorePoint 받아와서 하거나 수정 될거 같음
            switch (ClueItem->NoteType)
            {
            case ENoteType::Truth:
                TotalExplorePoint += 50;
                break;
            case ENoteType::Lie:
                TotalExplorePoint -= 20;
                break;
            case ENoteType::Noise:
            default:
                break;
            }
            //TotalExplorePoint += ClueItem->BaseExplorePoint;
            EXPDetails.Add(EXPInfo);
        }
    }

    // 6. 랭크 산정 : 로직 수정되어야 할거같음

    FString Rank = "C"; // Default fallback
    int32 TotalScore = TotalExplorePoint + ResourceScoreSum;

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
            if (TotalScore >= Row->MinScore)
            {
                Rank = Row->Rank;
                //TotalExplorePoint += Row->ExplorationPoint;
                break;
            }
        }

        LOG_Frame_WARNING(TEXT("랭크 판정 기준: 총점 %d → Rank %s"), TotalScore, *Rank);
    }
    else
    {
        LOG_Frame_WARNING(TEXT("RankThresholdTable이 설정되지 않았습니다. 기본 랭크 C로 설정됩니다."));
    }

    // 랭크에 따라 탐사포인트 지급후 최종 스코어 다시 산정
    //TotalScore = TotalExplorePoint + ResourceScoreSum;

    // 6. 결과 패킹
    FGameResultData Result;
    Result.CorrectChecklistCount = CorrectCount;
    Result.TotalChecklistCount = TotalChecklistCount;
    Result.CollectedResourcePoints = ResourceScoreSum;
    Result.ResourceScoreDetails = ResourceScoreDetails;
    Result.FinalScore = TotalScore;
    Result.Rank = Rank;

    Result.bIsSurvive = bIsSurvive;
    Result.ExplorePointDetails = EXPDetails;
    Result.FinalExporePoint = TotalExplorePoint;

    LOG_Frame_WARNING(TEXT("EvaluateResult 완료 - 정답:%d/%d 자원점수:%d → 총점:%d 랭크:%s"),
        CorrectCount,
        TotalChecklistCount,
        ResourceScoreSum,
        TotalScore,
        *Rank);

    return Result;
}