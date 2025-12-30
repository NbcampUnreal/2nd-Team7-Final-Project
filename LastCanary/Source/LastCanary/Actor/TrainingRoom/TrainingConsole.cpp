#include "Actor/TrainingRoom/TrainingConsole.h"
#include "Actor/TrainingRoom/TrainingRoomManager.h"
#include "Character/BaseCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ATrainingConsole::ATrainingConsole()
{
	PrimaryActorTick.bCanEverTick = false;

    ConsoleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConsoleMesh"));
    RootComponent = ConsoleMesh;
}

void ATrainingConsole::BeginPlay()
{
	Super::BeginPlay();
	
    if (!TrainingManager)
    {
        FindTrainingManager();
    }
}

void ATrainingConsole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATrainingConsole, PendingAction);
}

void ATrainingConsole::ExecutePendingAction(APlayerController* Interactor)
{
    switch (PendingAction)
    {
    case 0: // 시작
        StartTraining(Interactor);
        LOG_Item_WARNING(TEXT("[TrainingConsole] 대기 동작 실행: 훈련 시작"));
        break;
    case 1: // 중지
        StopTraining();
        LOG_Item_WARNING(TEXT("[TrainingConsole] 대기 동작 실행: 훈련 중지"));
        break;
    case 2: // 리셋
        ResetDummies();
        LOG_Item_WARNING(TEXT("[TrainingConsole] 대기 동작 실행: 더미 리셋"));
        break;
    }
}

void ATrainingConsole::FindTrainingManager()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATrainingRoomManager::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        TrainingManager = Cast<ATrainingRoomManager>(FoundActors[0]);
        LOG_Item_WARNING(TEXT("[TrainingConsole] 훈련장 매니저 자동 연결 완료: %s"), *TrainingManager->GetName());
    }
    else 
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole] 훈련장 매니저를 찾을 수 없습니다!"));
    }
}

void ATrainingConsole::Interact_Implementation(APlayerController* Interactor)
{
    if (!TrainingManager)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole] 훈련장 매니저가 연결되지 않음"));
        return;
    }

    if (HasAuthority() && PendingAction != 255)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole::Interact_Implementation] 서버에서 PendingAction 실행: %d"), PendingAction);
        ExecutePendingAction(Interactor);
        PendingAction = 255;
        return;
    }

    if (Interactor && Interactor->IsLocalPlayerController())
    {
        TArray<UUserWidget*> FoundWidgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UTrainingConsoleWidget::StaticClass());
        if (FoundWidgets.Num() > 0)
        {
            LOG_Item_WARNING(TEXT("[TrainingConsole] 이미 UI가 열려있음"));
            return;
        }

        LOG_Item_WARNING(TEXT("[TrainingConsole] UI 생성 - 플레이어 %s"), *Interactor->GetName());

        if (ConsoleWidgetClass)
        {
            UTrainingConsoleWidget* ConsoleWidget = CreateWidget<UTrainingConsoleWidget>(Interactor, ConsoleWidgetClass);
            if (ConsoleWidget)
            {
                ConsoleWidget->SetOwningConsole(this);
                ConsoleWidget->AddToViewport(10);

                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(ConsoleWidget->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                Interactor->SetInputMode(InputMode);
                Interactor->bShowMouseCursor = true;
            }
        }
    }
}

FString ATrainingConsole::GetInteractMessage_Implementation() const
{
    if (!TrainingManager)
    {
        return FString(TEXT("훈련장 콘솔 (매니저 연결 안됨)"));
    }

    // 현재 훈련 상태에 따라 메시지 변경
    if (TrainingManager->bSessionActive)
    {
        return FString(TEXT("[F] 훈련 중단"));
    }
    else
    {
        return FString(TEXT("[F] 훈련 시작"));
    }
}

void ATrainingConsole::StartTraining(APlayerController* Interactor)
{
    // 서버 권한 체크
    if (!HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole] 클라이언트에서 호출됨 - 무시"));
        return;
    }

    if (!TrainingManager)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole] 훈련장 매니저가 없어 시작 불가"));
        return;
    }

    if (!Interactor)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole] 유효하지 않은 플레이어 컨트롤러"));
        return;
    }

    // 플레이어 캐릭터 가져오기
    ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(Interactor->GetPawn());
    if (!PlayerCharacter)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole] 플레이어 캐릭터를 찾을 수 없음"));
        return;
    }

    // 설정 적용
    ApplySettings();

    // 훈련 시작 (플레이어 전달)
    TrainingManager->StartTrainingSession(PlayerCharacter);
    LOG_Item_WARNING(TEXT("[TrainingConsole] 훈련 시작됨 - 플레이어: %s"), *PlayerCharacter->GetName());
}

void ATrainingConsole::ServerStartTraining_Implementation(APlayerController* Interactor)
{
    if (!TrainingManager || !Interactor)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole] ServerStartTraining: 유효하지 않은 매니저 또는 플레이어"));
        return;
    }

    // 플레이어 캐릭터 가져오기
    ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(Interactor->GetPawn());
    if (!PlayerCharacter)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsole] 플레이어 캐릭터를 찾을 수 없음"));
        return;
    }

    ApplySettings();

    // 서버에서 훈련 시작
    TrainingManager->StartTrainingSession(PlayerCharacter);
    LOG_Item_WARNING(TEXT("[TrainingConsole] 훈련 시작됨 - 플레이어: %s"), *PlayerCharacter->GetName());
}

void ATrainingConsole::StopTraining()
{
    if (!TrainingManager)
    {
        return;
    }

    TrainingManager->EndTrainingSession();
    LOG_Item_WARNING(TEXT("[TrainingConsole] 훈련 종료됨"));
}

void ATrainingConsole::ResetDummies()
{
    if (!TrainingManager)
    {
        return;
    }

    TrainingManager->ClearAllDummies();
    TrainingManager->SpawnDummies(DummyCount);
    LOG_Item_WARNING(TEXT("[TrainingConsole] 더미 리셋 완료"));
}

void ATrainingConsole::ApplySettings()
{
    if (!TrainingManager)
    {
        return;
    }

    // 설정값을 매니저에 전달
    TrainingManager->SessionDuration = TrainingDuration;
    TrainingManager->CurrentTrainingMode = CurrentMode;
    TrainingManager->CurrentDummyCount = DummyCount;

    LOG_Item_WARNING(TEXT("[TrainingConsole] 설정 적용 - 모드: %d, 시간: %.1f초, 더미: %d개"), static_cast<int32>(CurrentMode), TrainingDuration, DummyCount);
}
