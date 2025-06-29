#include "Framework/Manager/ChecklistManager.h"
#include "Framework/Manager/ResultEvaluator.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Framework/GameMode/LCInGameModeBase.h"
#include "Framework/PlayerController/LCInGamePlayerController.h"
#include "Character/BasePlayerState.h"
#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIElement/ResultMenu.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DataTable/ItemDataRow.h"
#include "DataTable/RankThresholdRow.h"
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
}

void AChecklistManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AChecklistManager::InitCheckListManager(UDataTable* CheckListTable)
{
	Questions.Empty();
	CorrectAnswers.Empty();
	TotalPlayerCount = GetNumPlayers();
	SubmittedCount = 0;

	ALCGameMode* InGameMode = GetWorld()->GetAuthGameMode<ALCGameMode>();
	if (InGameMode)
	{
		for (auto PC : InGameMode->AllPlayerControllers)
		{
			NewPlayerResults.Add(PC, FPlayerResultData());
		}
	}

	ChecklistDataTable = CheckListTable;

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

void AChecklistManager::StartChecklist()
{
	LOG_Frame_WARNING(TEXT("StartChecklist."));
	TotalPlayerCount = GetNumPlayers();
	SubmittedCount = 0;
	LOG_Frame_WARNING(TEXT("제출해야할 플레이어의 숫자 = %d."), TotalPlayerCount); // << 서버
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
	// TODO : GameManger로 부터 받아오기
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

FString AChecklistManager::GetMVPName()
{
	int32 HighestScore = -1;
	FString TopPlayerName = TEXT("Unknown");

	for (const TPair<APlayerController*, FPlayerResultData>& Pair : NewPlayerResults)
	{
		const APlayerController* Controller = Pair.Key;
		const FPlayerResultData& Result = Pair.Value;

		if (Result.TotalScore > HighestScore && Controller && Controller->PlayerState)
		{
			HighestScore = Result.TotalScore;
			TopPlayerName = Controller->PlayerState->GetPlayerName();
		}
	}

	return TopPlayerName;
}

int32 AChecklistManager::GetTotalResourcePoint()
{
	int32 TotalResource = 0;

	for (const TPair<APlayerController*, FPlayerResultData>& Pair : NewPlayerResults)
	{
		TotalResource += Pair.Value.ResourcePoint;
	}

	return TotalResource;
}

int32 AChecklistManager::GetTotalEXP()
{
	int32 TotalEXP = 0;

	for (const TPair<APlayerController*, FPlayerResultData>& Pair : NewPlayerResults)
	{
		TotalEXP += Pair.Value.ExplorePoint;
	}

	return TotalEXP;
}

void AChecklistManager::AddOrUpdatePlayerResult(APlayerController* Submitter, const FPlayerResultData& PlayerResultData)
{
	if (NewPlayerResults.Contains(Submitter))
	{
		NewPlayerResults[Submitter] = PlayerResultData;
	}
	else
	{
		NewPlayerResults.Add(Submitter, PlayerResultData);
	}
}

bool AChecklistManager::IsAllPlayerSubmitCheckList()
{
	return SubmittedCount >= TotalPlayerCount;
}

void AChecklistManager::SubmitCheckList(APlayerController* Submitter, const TArray<FChecklistQuestion>& PlayerAnswers)
{
	//Server_SubmitChecklist_Implementation(Submitter, PlayerAnswers);

	if (!IsValid(Submitter))
	{
		LOG_Game_WARNING(TEXT("[ChecklistManager] Invalid Submitter"));
		return;
	}

	if (!NewPlayerResults.Contains(Submitter))
	{
		LOG_Game_ERROR(TEXT("[ChecklistManager] Can't Not Find Submitter Player Controller"));
		return;
	}

	if (!Evaluator)
	{
		LOG_Game_WARNING(TEXT("[ChecklistManager] Evaluator is null"));
		return;
	}

	ABasePlayerState* SubmitterPS = Cast<ABasePlayerState>(Submitter->PlayerState);
	if (!IsValid(SubmitterPS))
	{
		LOG_Game_WARNING(TEXT("[ChecklistManager] Submitter PlayState Casting Fail"));
	}


	bool bIsSurvive = SubmitterPS->CurrentState == EPlayerState::Dead ? false : true;
	TMap<FName, int32> ParsedResources = SubmitterPS->CollectedResourceMap;
	TArray<int32> ExpItems = SubmitterPS->CollectedExploreItemArray;

	FGameResultData GameResult = Evaluator->EvaluatePlayerResult
	(
		PlayerAnswers,
		CorrectAnswers,
		bIsSurvive,
		SubmitterPS->SurviveTime,
		SubmitterPS->KillCount,
		ParsedResources,
		ExpItems
	);

	LOG_Frame_WARNING(TEXT("[ChecklistManager] EvaluateResult 결과 → 정답: %d / %d | 생존: %d | 자원점수 항목 수: %d | 단서 항목 수: %d | 최종점수: %d | 랭크: %s"),
		GameResult.CorrectChecklistCount,
		GameResult.TotalChecklistCount,
		bIsSurvive,
		GameResult.ResourceScoreDetails.Num(),
		GameResult.ExplorePointDetails.Num(),
		GameResult.FinalScore,
		*GameResult.Rank);

	// =============================================================================
	FPlayerResultData PlayerResultData;
	PlayerResultData.OwnerController = Submitter;
	PlayerResultData.PlayerName = Submitter->PlayerState->GetPlayerName();
	PlayerResultData.bIsSurvived = bIsSurvive;
	PlayerResultData.CorrectRate = (float)GameResult.CorrectChecklistCount / GameResult.TotalChecklistCount;
	PlayerResultData.SurviveTime = SubmitterPS->SurviveTime;
	PlayerResultData.KillCount = SubmitterPS->KillCount;

	// 재화
	PlayerResultData.ResourceDetails = GameResult.ResourceScoreDetails;
	PlayerResultData.ResourcePoint = GameResult.CollectedResourcePoints;
	PlayerResultData.TotalScore = GameResult.FinalScore;

	// 탐사 포인트
	PlayerResultData.ExplorePointDetails = GameResult.ExplorePointDetails;
	PlayerResultData.ExplorePoint = GameResult.FinalExporePoint;

	PlayerResultData.Rank = GameResult.Rank;

	AddOrUpdatePlayerResult(Submitter, PlayerResultData);
	// =============================================================================

	SubmittedCount++;

	LOG_Game_WARNING(TEXT("[ChecklistManager] %s 결과 저장 완료 (%d / %d)"), *Submitter->GetName(), SubmittedCount, TotalPlayerCount);

	// 모든 플레이어가 제출 완료 평가 시작
	if (IsAllPlayerSubmitCheckList())
	{
		LOG_Game_WARNING(TEXT("[ChecklistManager] All Players Submit Check List!! Start Result!!"));
		StartResult();
	}
}

void AChecklistManager::StartResult()
{
	if (!HasAuthority())
	{
		return;
	}

	SetTotalGameResult();

	// Client Show Result Widget
	for (auto PlayerResult : NewPlayerResults)
	{
		//TotalGameResult.PlayersResult.Add(PlayerResult.Value);
		if (ALCInGamePlayerController* InGamePC = Cast<ALCInGamePlayerController>(PlayerResult.Key))
		{
			InGamePC->Client_ShowResultWidget(TotalGameResult);
		}
	}
}

void AChecklistManager::SetTotalGameResult()
{
	ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>();
	if (!IsValid(LCGM))
	{
		LOG_Game_WARNING(TEXT("[ChecklistManager] GameManager Casting Fail"));
		return;
	}

	TotalGameResult.CurrentRound = LCGM->CurrentGamePlayData.Round;
	TotalGameResult.CurrentMap = LCGM->CurrentGamePlayData.MapName;
	TotalGameResult.MVPName = GetMVPName();
	TotalGameResult.TotalReources = GetTotalResourcePoint();
	TotalGameResult.Payment = LCGM->GetPayMent();
	TotalGameResult.TotalEXP = GetTotalEXP();

	int32 TotalResource = GetTotalResourcePoint();
	int32 TotalEXP = GetTotalEXP();
	int32 TotalScore = TotalResource + TotalEXP;

	FString TotalRank = TEXT("C");
	// TO DO : Total Rank 점수 산정 로직 필요
	if (TotalRankTable)
	{
		TArray<FRankThresholdRow*> RankRows;
		TotalRankTable->GetAllRows(TEXT("Rank Lookup"), RankRows);

		// 점수 높은 순서로 정렬
		RankRows.Sort([](const FRankThresholdRow& A, const FRankThresholdRow& B) {
			return A.MinScore > B.MinScore;
			});

		for (const auto* Row : RankRows)
		{
			if (TotalScore >= Row->MinScore * TotalPlayerCount)
			{
				TotalRank = Row->Rank;
				//TotalEXP += Row->ExplorationPoint;
				break;
			}
		}
	}

	TotalGameResult.TotalRank = TotalRank;

	for (auto PlayerResult : NewPlayerResults)
	{
		TotalGameResult.PlayersResult.Add(PlayerResult.Value);
	}
}



void AChecklistManager::Server_SubmitChecklist_Implementation(APlayerController* Submitter, const TArray<FChecklistQuestion>& PlayerAnswers)
{
	if (!IsValid(Submitter))
	{
		LOG_Game_WARNING(TEXT("[ChecklistManager] Invalid Submitter"));
		return;
	}

	if (!Evaluator)
	{
		LOG_Game_WARNING(TEXT("[ChecklistManager] Evaluator is null"));
		return;
	}

	ABasePlayerState* SubmitterPS = Cast<ABasePlayerState>(Submitter->PlayerState);
	if (!IsValid(SubmitterPS))
	{
		LOG_Game_WARNING(TEXT("[ChecklistManager] Submitter PlayState Casting Fail"));
	}

	bool bIsSurvive = SubmitterPS->CurrentState == EPlayerState::Dead ? false : true;

	TotalPlayerCount = GetNumPlayers();

	// 생존자 수 계산
	int32 SurvivingCount = 0;
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (const ABasePlayerState* BasePS = Cast<ABasePlayerState>(PS))
		{
			if (BasePS->CurrentState != EPlayerState::Dead)
			{
				SurvivingCount++;
			}

			//if (BasePS->bHasEscaped)
			//{
			//	++SurvivingCount;
			//}
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

	// ==========================================================================

	FChecklistResultData FinalResult;
	FinalResult.OwnerController = Submitter;
	FinalResult.CorrectRate = (float)GameResult.CorrectChecklistCount / GameResult.TotalChecklistCount;
	FinalResult.bIsSurvived = bIsSurvive;
	FinalResult.Score = GameResult.FinalScore;
	FinalResult.Rank = GameResult.Rank;
	FinalResult.ResourceDetails = GameResult.ResourceScoreDetails;

	PlayerResults.Add(Submitter, FinalResult);

	// =============================================================================

	if (ABasePlayerState* PS = Cast<ABasePlayerState>(Submitter->PlayerState))
	{
		PS->AddTotalGold(FinalResult.Score);
		LOG_Frame_WARNING(TEXT("[ChecklistManager] %s 골드 보상 지급: +%d (총 골드: %d)"),
			*Submitter->GetName(), FinalResult.Score, PS->GetTotalGold());
	}

	SubmittedCount++;
	LOG_Frame_WARNING(TEXT("[ChecklistManager] %s 결과 저장 완료 (%d / %d)"),
		*Submitter->GetName(), SubmittedCount, TotalPlayerCount);  // << 여기 왜 0임?

	if (SubmittedCount >= TotalPlayerCount)
	{
		StartResult();
	}
}
