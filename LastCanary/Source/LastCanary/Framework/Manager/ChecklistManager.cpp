#include "Framework/Manager/ChecklistManager.h"
#include "Framework/Manager/ResultEvaluator.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Character/BasePlayerState.h"
#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIElement/ResultMenu.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DataTable/ItemDataRow.h"
#include "Engine/World.h"

#include "LastCanary.h"

AChecklistManager::AChecklistManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AChecklistManager::BeginPlay()
{
	Super::BeginPlay();
	Questions.Empty();
	CorrectAnswers.Empty();

	if (!Evaluator)
	{
		Evaluator = NewObject<UResultEvaluator>(this, UResultEvaluator::StaticClass());
		LOG_Frame_WARNING(TEXT("Evaluator 객체 생성 완료."));

		Evaluator->ResourceItemTable = ResourceItemTable;
		Evaluator->ResourceCategoryTable = ResourceCategoryTable;
		Evaluator->RankThresholdTable = RankThresholdTable;
	}

	if (ChecklistDataTable)
	{
		static const FString ContextString(TEXT("Checklist Load"));
		TArray<FChecklistQuestionRow*> Rows;
		ChecklistDataTable->GetAllRows(ContextString, Rows);

		for (auto* Row : Rows)
		{
			if (Row)
			{
				FChecklistQuestion Q;
				Q.QuestionText = Row->QuestionText;
				Q.bAnswer = false;
				Q.bIsAnswered = false;

				Questions.Add(Q);
				CorrectAnswers.Add(Row->bCorrectAnswer);
			}
		}
	}
}

void AChecklistManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AChecklistManager::StartChecklist()
{
	LOG_Frame_WARNING(TEXT("StartChecklist."));
	TotalPlayerCount = GetNumPlayers();
	SubmittedCount = 0;

	if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		LOG_Frame_WARNING(TEXT("GISubsystem 있음."));
		if (ULCUIManager* UIManager = GISubsystem->GetUIManager())
		{
			LOG_Frame_WARNING(TEXT("UIManager 있음."));
			UIManager->ShowChecklistWidget();
			if (UChecklistWidget* Widget = UIManager->GetChecklistWidget())
			{
				LOG_Frame_WARNING(TEXT("Widget->InitWithQuestions(Questions, this);."));

				Widget->InitWithQuestions(Questions, this);
			}
		}
	}
}

int32 AChecklistManager::GetNumPlayers() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetGameState()->PlayerArray.Num();
	}
	return 0;
}

TMap<FName, int32> AChecklistManager::CollectAllPlayerResources()
{
	TMap<FName, int32> MergedResources;

	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (ABasePlayerState* BasePS = Cast<ABasePlayerState>(PS))
			{
				const TMap<FName, int32>& PlayerMap = BasePS->GetCollectedResourceMap();
				LOG_Frame_WARNING(TEXT("Player %s has %d items"), *BasePS->GetPlayerName(), PlayerMap.Num());

				for (const auto& Elem : PlayerMap)
				{
					LOG_Frame_WARNING(TEXT("  - %s : %d"), *Elem.Key.ToString(), Elem.Value);
					MergedResources.FindOrAdd(Elem.Key) += Elem.Value;
				}
			}
		}
	}
	else
	{
		LOG_Frame_WARNING(TEXT("[ChecklistManager] GameState is null!"));
	}

	return MergedResources;
}

void AChecklistManager::Server_SubmitChecklist_Implementation(APlayerController* Submitter, const TArray<FChecklistQuestion>& PlayerAnswers)
{
	if (!IsValid(Submitter))
	{
		LOG_Frame_WARNING(TEXT("[ChecklistManager] Invalid Submitter"));
		return;
	}

	if (!Evaluator)
	{
		LOG_Frame_WARNING(TEXT("[ChecklistManager] Evaluator is null"));
		return;
	}

	// 생존자 수 계산
	int32 SurvivingCount = 0;
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (const ABasePlayerState* BasePS = Cast<ABasePlayerState>(PS))
		{
			if (BasePS->bHasEscaped)
			{
				++SurvivingCount;
			}
		}
	}

	TMap<FName, int32> ParsedResources;
	if (ResourceItemTable)
	{
		TMap<FName, int32> AllResources = CollectAllPlayerResources();
		LOG_Frame_WARNING(TEXT("[ChecklistManager] ResourceItemTable 있음. 리소스 파싱 시작."));
		LOG_Frame_WARNING(TEXT("[ChecklistManager] 전체 자원 수집 개수: %d"), AllResources.Num());

		for (const TPair<FName, int32>& Pair : AllResources)
		{
			const FName& RowName = Pair.Key;
			int32 Count = Pair.Value;

			const FItemDataRow* Row = ResourceItemTable->FindRow<FItemDataRow>(RowName, TEXT("Checklist Resource Parse"));
			if (Row == nullptr)
			{
				LOG_Frame_WARNING(TEXT("[ChecklistManager] ResourceItemTable에서 %s 항목을 찾을 수 없습니다."), *RowName.ToString());
				continue;
			}

			if (Row->bIsResourceItem && Count > 0)
			{
				LOG_Frame_WARNING(TEXT("[ChecklistManager] %s 항목 파싱 완료. 개수: %d"), *RowName.ToString(), Count);
				ParsedResources.Add(RowName, Count);
			}
		}
	}

	FGameResultData GameResult = Evaluator->EvaluateResult(
		PlayerAnswers,
		CorrectAnswers,
		SurvivingCount,
		ParsedResources
	);

	LOG_Frame_WARNING(TEXT("[ChecklistManager] EvaluateResult 결과 → 정답: %d / %d | 생존: %d | 자원점수 항목 수: %d | 최종점수: %d | 랭크: %s"),
		GameResult.CorrectChecklistCount,
		GameResult.TotalChecklistCount,
		SurvivingCount,
		GameResult.ResourceScoreDetails.Num(),
		GameResult.FinalScore,
		*GameResult.Rank);

	FChecklistResultData FinalResult;
	FinalResult.OwnerController = Submitter;
	FinalResult.CorrectRate = (float)GameResult.CorrectChecklistCount / GameResult.TotalChecklistCount;
	FinalResult.bIsSurvived = true;
	FinalResult.Score = GameResult.FinalScore;
	FinalResult.Rank = GameResult.Rank;
	FinalResult.ResourceDetails = GameResult.ResourceScoreDetails;

	PlayerResults.Add(Submitter, FinalResult);

	if (ABasePlayerState* PS = Cast<ABasePlayerState>(Submitter->PlayerState))
	{
		PS->AddTotalGold(FinalResult.Score);
		LOG_Frame_WARNING(TEXT("[ChecklistManager] %s 골드 보상 지급: +%d (총 골드: %d)"),
			*Submitter->GetName(), FinalResult.Score, PS->GetTotalGold());
	}

	SubmittedCount++;
	LOG_Frame_WARNING(TEXT("[ChecklistManager] %s 결과 저장 완료 (%d / %d)"),
		*Submitter->GetName(), SubmittedCount, TotalPlayerCount);

	if (SubmittedCount >= TotalPlayerCount)
	{
		LOG_Frame_WARNING(TEXT("[ChecklistManager] 모든 플레이어 제출 완료. 결과 전송 시작."));

		for (const TPair<APlayerController*, FChecklistResultData>& Pair : PlayerResults)
		{
			if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(Pair.Key))
			{
				RoomPC->Client_NotifyResultReady(Pair.Value);
			}
		}
	}
}
