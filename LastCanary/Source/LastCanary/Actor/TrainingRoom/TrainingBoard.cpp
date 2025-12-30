#include "Actor/TrainingRoom/TrainingBoard.h"
#include "Actor/TrainingRoom/TrainingRoomManager.h"
#include "UI/UIElement/TrainingBoardWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "LastCanary.h"

ATrainingBoard::ATrainingBoard()
{
	PrimaryActorTick.bCanEverTick = false;

    // 루트 컴포넌트
    BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
    RootComponent = BoardMesh;
    BoardMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 위젯 컴포넌트
    StatsWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatsWidget"));
    StatsWidget->SetupAttachment(RootComponent);
    StatsWidget->SetWidgetSpace(EWidgetSpace::World);
    StatsWidget->SetDrawSize(FVector2D(1000.0f, 600.0f));
    StatsWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

    bReplicates = true;
    bAlwaysRelevant = true;

    BoardWidgetInstance = nullptr;
    ElapsedTime = 0.0f;
}

void ATrainingBoard::BeginPlay()
{
	Super::BeginPlay();
	
    // 매니저가 설정되지 않았으면 자동으로 찾기
    if (!TrainingManager)
    {
        FindTrainingManager();
    }

    // 매니저 이벤트 바인딩
    if (TrainingManager)
    {
        TrainingManager->OnStatsUpdated.AddDynamic(this, &ATrainingBoard::OnStatsUpdated);
        TrainingManager->OnSessionStateChanged.AddDynamic(this, &ATrainingBoard::OnSessionStateChanged);

        LOG_Item_WARNING(TEXT("[TrainingBoard] 훈련장 매니저 연결 완료"));
    }

    // 위젯 클래스 설정
    if (StatsWidgetClass && StatsWidget)
    {
        StatsWidget->SetWidgetClass(StatsWidgetClass);

        BoardWidgetInstance = Cast<UTrainingBoardWidget>(StatsWidget->GetWidget());

        if (BoardWidgetInstance)
        {
            BoardWidgetInstance->ShowIdleState();
        }
    }

    // 주기적 업데이트 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &ATrainingBoard::PeriodicUpdate, UpdateInterval, true);
}

void ATrainingBoard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    if (TrainingManager)
    {
        TrainingManager->OnStatsUpdated.RemoveDynamic(this, &ATrainingBoard::OnStatsUpdated);
        TrainingManager->OnSessionStateChanged.RemoveDynamic(this, &ATrainingBoard::OnSessionStateChanged);
    }

    Super::EndPlay(EndPlayReason);
}

void ATrainingBoard::FindTrainingManager()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATrainingRoomManager::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        TrainingManager = Cast<ATrainingRoomManager>(FoundActors[0]);
        LOG_Item_WARNING(TEXT("[TrainingBoard] 훈련장 매니저 자동 연결: %s"), *TrainingManager->GetName());
    }
    else
    {
        LOG_Item_WARNING(TEXT("[TrainingBoard] 훈련장 매니저를 찾을 수 없음"));
    }
}

void ATrainingBoard::OnStatsUpdated(const FTrainingStats& Stats)
{
    if (BoardWidgetInstance && TrainingManager)
    {
        BoardWidgetInstance->UpdateStats(Stats, TrainingManager->bSessionActive, TrainingManager->CurrentTrainingMode);
    }

    UpdateStatsDisplay(Stats);
}

void ATrainingBoard::OnSessionStateChanged(bool bActive)
{
    if (!BoardWidgetInstance || !TrainingManager)
    {
        return;
    }

    if (bActive)
    {
        ElapsedTime = 0.0f;
        BoardWidgetInstance->OnSessionStart(TrainingManager->CurrentTrainingMode);
        OnSessionStarted();
    }
    else
    {
        BoardWidgetInstance->OnSessionEnd(TrainingManager->GetCurrentStats());
        OnSessionEnded(TrainingManager->GetCurrentStats());
    }
}

void ATrainingBoard::PeriodicUpdate()
{
    if (!TrainingManager || !BoardWidgetInstance)
    {
        return;
    }

    if (!TrainingManager->bSessionActive)
    {
        return;
    }

    ElapsedTime += UpdateInterval;

    FTrainingStats CurrentStats = TrainingManager->GetCurrentStats();
    CurrentStats.SessionTime = GetWorld()->GetTimeSeconds() - TrainingManager->SessionStartTime;

    BoardWidgetInstance->UpdateStats(CurrentStats, true, TrainingManager->CurrentTrainingMode);
    UpdateStatsDisplay(CurrentStats);
}

