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

	// 자원 데이터 수집 (Submitter가 가진 CollectedResources 사용, 예시)
	// TMap<FName, int32> CollectedResources;

	const TArray<FName> ResourceIDMapping = {
	TEXT("AncientRuneStone"),
	TEXT("RadiantFragment"),
	TEXT("SealedMask")
	};

	TMap<FName, int32> ParsedResources;

	if (ABasePlayerState* PS = Cast<ABasePlayerState>(Submitter->PlayerState))
	{
		// TODO : 실제 자원 데이터 가져오기
		// 아이템 획득 시 CollectedResources에 추가되도록 구현 필요
		// 예시로 몇 가지 자원 추가
		if (PS->CollectedResources.Num() == 0)
		{
			PS->CollectedResources.Init(0, ResourceIDMapping.Num());
			PS->CollectedResources[0] = 100;
			PS->CollectedResources[1] = 5;
			PS->CollectedResources[2] = 30;
		}

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

//void AChecklistManager::Client_NotifyResultReady_Implementation(const FGameResultData& ResultData)
//{
//	LOG_Frame_WARNING(TEXT("Client_NotifyResultReady → ShowResultMenu"));
//
//	if (ResultMenuClass)
//	{
//		if (UResultMenu* Widget = CreateWidget<UResultMenu>(GetWorld(), ResultMenuClass))
//		{
//			Widget->AddToViewport();
//
//			TArray<FResultRewardEntry> RewardList;
//			RewardList.Add({ FText::FromString("Checklist"), FText::Format(FText::FromString("Accuracy: {0}/{1}"), FText::AsNumber(ResultData.CorrectChecklistCount), FText::AsNumber(ResultData.TotalChecklistCount)), ResultData.CorrectChecklistCount * 100 });
//			RewardList.Add({ FText::FromString("Survivors"), FText::FromString(ResultData.SurvivingPlayerCount > 0 ? "Survived" : "Dead"), ResultData.SurvivingPlayerCount * 50 });
//			RewardList.Add({ FText::FromString("Resources"), FText::AsNumber(ResultData.CollectedResourcePoints), ResultData.CollectedResourcePoints });
//
//			Widget->SetRewardEntries(RewardList);
//			Widget->SetResourceScoreDetails(ResultData.ResourceScoreDetails);
//			Widget->SetTotalGold(ResultData.FinalScore);
//			Widget->SetRankText(ResultData.Rank);
//		}
//	}
//}
//
//void AChecklistManager::Server_NotifyRevealFinished_Implementation(APlayerController* Controller)
//{
//	if (PlayerResults.Contains(Controller))
//	{
//		LOG_Frame_WARNING(TEXT("[ChecklistManager] 플레이어 %s의 결과를 클라이언트로 전송합니다."), *Controller->GetActorNameOrLabel());
//		// Controller의 소유자 클라이언트로 전달
//		Client_ShowResultMenu(PlayerResults[Controller]);
//	}
//	else
//	{
//		LOG_Frame_WARNING(TEXT("[ChecklistManager] 플레이어 %s의 결과가 없습니다!"), *Controller->GetActorNameOrLabel());
//	}
//}
//
//void AChecklistManager::Client_ShowResultMenu_Implementation(const FGameResultData& ResultData)
//{
//	if (ResultMenuClass)
//	{
//		if (UResultMenu* Widget = CreateWidget<UResultMenu>(GetWorld(), ResultMenuClass))
//		{
//			Widget->AddToViewport(999); // 높은 ZOrder
//			const bool bInViewport = Widget->IsInViewport();
//
//			LOG_Frame_WARNING(TEXT("[ChecklistManager] 결과 위젯 생성 완료 → IsInViewport: %s"), bInViewport ? TEXT("true") : TEXT("false"));
//
//			TArray<FResultRewardEntry> RewardList;
//			RewardList.Add({ FText::FromString("Checklist"),
//							 FText::Format(FText::FromString("Accuracy: {0}/{1}"),
//								FText::AsNumber(ResultData.CorrectChecklistCount),
//								FText::AsNumber(ResultData.TotalChecklistCount)),
//							 ResultData.CorrectChecklistCount * 100 });
//
//			RewardList.Add({ FText::FromString("Survivors"),
//							 FText::FromString(ResultData.SurvivingPlayerCount > 0 ? "Survived" : "Dead"),
//							 ResultData.SurvivingPlayerCount * 50 });
//
//			RewardList.Add({ FText::FromString("Resources"),
//							 FText::AsNumber(ResultData.CollectedResourcePoints),
//							 ResultData.CollectedResourcePoints });
//
//			LOG_Frame_WARNING(TEXT("[ChecklistManager] Checklist 정답 수: %d / %d"), ResultData.CorrectChecklistCount, ResultData.TotalChecklistCount);
//			LOG_Frame_WARNING(TEXT("[ChecklistManager] 생존 여부: %d"), ResultData.SurvivingPlayerCount);
//			LOG_Frame_WARNING(TEXT("[ChecklistManager] 자원 점수: %d"), ResultData.CollectedResourcePoints);
//			LOG_Frame_WARNING(TEXT("[ChecklistManager] 총 점수: %d, 랭크: %s"), ResultData.FinalScore, *ResultData.Rank);
//
//			Widget->SetRewardEntries(RewardList);
//			Widget->SetResourceScoreDetails(ResultData.ResourceScoreDetails);
//			Widget->SetTotalGold(ResultData.FinalScore);
//			Widget->SetRankText(ResultData.Rank);
//
//			// UI 전용 입력 모드 설정
//			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
//			{
//				FInputModeUIOnly InputMode;
//				InputMode.SetWidgetToFocus(Widget->TakeWidget());
//				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
//				PC->SetInputMode(InputMode);
//				PC->bShowMouseCursor = true;
//
//				LOG_Frame_WARNING(TEXT("[ChecklistManager] 입력 모드 UI 전용으로 설정 완료."));
//			}
//		}
//		else
//		{
//			LOG_Frame_WARNING(TEXT("[ChecklistManager] 결과 위젯 생성 실패!"));
//		}
//	}
//	else
//	{
//		LOG_Frame_WARNING(TEXT("[ChecklistManager] ResultMenuClass가 nullptr입니다."));
//	}
//}