#include "Framework/Manager/ResultEvaluator.h"

FGameResultData UResultEvaluator::EvaluateResult(const TArray<FChecklistQuestion>& PlayerAnswers,
    const TArray<bool>& CorrectAnswers,
    int32 SurvivingPlayers,
    int32 ResourcePoints)
{
    int32 CorrectCount = 0;

    for (int32 i = 0; i < FMath::Min(PlayerAnswers.Num(), CorrectAnswers.Num()); ++i)
    {
        if (PlayerAnswers[i].bIsAnswered && PlayerAnswers[i].bAnswer == CorrectAnswers[i])
        {
            ++CorrectCount;
        }
    }

    const int32 Total = CorrectAnswers.Num();
    int32 Score = CorrectCount * 100 + SurvivingPlayers * 50 + ResourcePoints;

    FString Rank;
    if (Score >= 400) Rank = "S";
    else if (Score >= 300) Rank = "A";
    else if (Score >= 200) Rank = "B";
    else Rank = "C";

    FGameResultData Result;
    Result.CorrectChecklistCount = CorrectCount;
    Result.TotalChecklistCount = Total;
    Result.SurvivingPlayerCount = SurvivingPlayers;
    Result.CollectedResourcePoints = ResourcePoints;
    Result.FinalScore = Score;
    Result.Rank = Rank;

    return Result;
}
