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
	TotalPlayerCount = GetNumPlayers();
	SubmittedCount = 0;

	if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = GISubsystem->GetUIManager())
		{
			UIManager->ShowChecklistWidget();
			if (UChecklistWidget* Widget = UIManager->GetChecklistWidget())
			{
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

	const TArray<FName> ResourceIDMapping = {
	TEXT("AncientRuneStone"),
	TEXT("RadiantFragment"),
	TEXT("SealedMask")
	};

	TMap<FName, int32> ParsedResources;

	if (ABasePlayerState* PS = Cast<ABasePlayerState>(Submitter->PlayerState))
	{
		// 예시로 몇 가지 자원 추가
		//if (PS->CollectedResources.Num() == 0)
		//{
		//	PS->CollectedResources.Init(0, ResourceIDMapping.Num());

		//	PS->CollectedResources[0] = 100;
		//	PS->CollectedResources[1] = 5;
		//	PS->CollectedResources[2] = 30;
		//}

		for (int32 i = 0; i < PS->CollectedResources.Num(); ++i)
		{
			if (ResourceIDMapping.IsValidIndex(i))
			{
				ParsedResources.Add(ResourceIDMapping[i], PS->CollectedResources[i]);
			}
		}
	}

	FGameResultData GameResult = Evaluator->EvaluateResult(
		PlayerAnswers,
		CorrectAnswers,
		SurvivingCount,
		ParsedResources
//		CollectedResources
	);

	// 결과 저장
	FChecklistResultData FinalResult;
	FinalResult.OwnerController = Submitter;
	FinalResult.CorrectRate = (float)GameResult.CorrectChecklistCount / GameResult.TotalChecklistCount;
	FinalResult.bIsSurvived = true; // bHasEscaped 값으로 넣어도 OK
	FinalResult.Score = GameResult.FinalScore;
	FinalResult.Rank = GameResult.Rank;
	FinalResult.ResourceDetails = GameResult.ResourceScoreDetails;

	PlayerResults.Add(Submitter, FinalResult);

	if (ABasePlayerState* PS = Cast<ABasePlayerState>(Submitter->PlayerState))
	{
		// 점수 = 골드 보상
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