#include "UI/UIElement/TrainingConsoleWidget.h"
#include "Actor/TrainingRoom/TrainingConsole.h"
#include "Actor/TrainingRoom/TrainingRoomManager.h"
#include "Character/BaseCharacter.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"


void UTrainingConsoleWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (StartButton)
    {
        StartButton->OnClicked.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnStartButtonClicked);
    }
    if (StopButton)
    {
        StopButton->OnClicked.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnStopButtonClicked);
    }
    if (ResetButton)
    {
        ResetButton->OnClicked.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnResetButtonClicked);
    }
    if (CloseButton)
    {
        CloseButton->OnClicked.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnCloseButtonClicked);
    }

    if (ModeComboBox)
    {
        ModeComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnModeSelected);
    }
    if (DummyTypeComboBox)
    {
        DummyTypeComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnDummyTypeSelected);
    }

    if (DummyCountSlider)
    {
        DummyCountSlider->OnValueChanged.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnDummyCountChanged);
    }
    if (TimeSlider)
    {
        TimeSlider->OnValueChanged.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnTimeChanged);
    }
    if (MovementSpeedSlider)
    {
        MovementSpeedSlider->OnValueChanged.AddUniqueDynamic(this, &UTrainingConsoleWidget::OnMovementSpeedChanged);
    }

    PopulateComboBoxes();
    InitializeUI();
}

void UTrainingConsoleWidget::NativeDestruct()
{
    // 이벤트 언바인딩
    if (StartButton)
        StartButton->OnClicked.RemoveDynamic(this, &UTrainingConsoleWidget::OnStartButtonClicked);
    if (StopButton)
        StopButton->OnClicked.RemoveDynamic(this, &UTrainingConsoleWidget::OnStopButtonClicked);
    if (ResetButton)
        ResetButton->OnClicked.RemoveDynamic(this, &UTrainingConsoleWidget::OnResetButtonClicked);
    if (CloseButton)
        CloseButton->OnClicked.RemoveDynamic(this, &UTrainingConsoleWidget::OnCloseButtonClicked);

    if (ModeComboBox)
        ModeComboBox->OnSelectionChanged.RemoveDynamic(this, &UTrainingConsoleWidget::OnModeSelected);
    if (DummyTypeComboBox)
        DummyTypeComboBox->OnSelectionChanged.RemoveDynamic(this, &UTrainingConsoleWidget::OnDummyTypeSelected);

    if (DummyCountSlider)
        DummyCountSlider->OnValueChanged.RemoveDynamic(this, &UTrainingConsoleWidget::OnDummyCountChanged);
    if (TimeSlider)
        TimeSlider->OnValueChanged.RemoveDynamic(this, &UTrainingConsoleWidget::OnTimeChanged);
    if (MovementSpeedSlider)
        MovementSpeedSlider->OnValueChanged.RemoveDynamic(this, &UTrainingConsoleWidget::OnMovementSpeedChanged);

    Super::NativeDestruct();
}

void UTrainingConsoleWidget::SetOwningConsole(ATrainingConsole* Console)
{
    OwningConsole = Console;
    InitializeUI();
}

void UTrainingConsoleWidget::InitializeUI()
{
    if (!OwningConsole)
        return;

    // 콘솔의 현재 설정값으로 UI 초기화
    SelectedMode = OwningConsole->CurrentMode;
    SelectedDummyType = OwningConsole->DefaultDummyType;
    SelectedDummyCount = OwningConsole->DummyCount;
    SelectedDuration = OwningConsole->TrainingDuration;
    SelectedMovementSpeed = OwningConsole->DummyMovementSpeed;

    // 슬라이더 값 설정
    if (DummyCountSlider)
    {
        DummyCountSlider->SetValue(SelectedDummyCount);
        OnDummyCountChanged(SelectedDummyCount);
    }

    if (TimeSlider)
    {
        TimeSlider->SetValue(SelectedDuration);
        OnTimeChanged(SelectedDuration);
    }

    if (MovementSpeedSlider)
    {
        MovementSpeedSlider->SetValue(SelectedMovementSpeed);
        OnMovementSpeedChanged(SelectedMovementSpeed);
    }

    // 훈련 상태 확인
    bool bTrainingActive = OwningConsole->TrainingManager && OwningConsole->TrainingManager->bSessionActive;
    UpdateButtonStates(bTrainingActive);
}

void UTrainingConsoleWidget::UpdateButtonStates(bool bTrainingActive)
{
    if (StartButton)
        StartButton->SetIsEnabled(!bTrainingActive);
    if (StopButton)
        StopButton->SetIsEnabled(bTrainingActive);
    if (ResetButton)
        ResetButton->SetIsEnabled(!bTrainingActive);

    // 훈련 중에는 설정 변경 불가
    if (ModeComboBox)
        ModeComboBox->SetIsEnabled(!bTrainingActive);
    if (DummyTypeComboBox)
        DummyTypeComboBox->SetIsEnabled(!bTrainingActive);
    if (DummyCountSlider)
        DummyCountSlider->SetIsEnabled(!bTrainingActive);
    if (TimeSlider)
        TimeSlider->SetIsEnabled(!bTrainingActive);
    if (MovementSpeedSlider)
        MovementSpeedSlider->SetIsEnabled(!bTrainingActive);
}

void UTrainingConsoleWidget::PopulateComboBoxes()
{
    // 훈련 모드 옵션
    if (ModeComboBox)
    {
        ModeComboBox->ClearOptions();
        ModeComboBox->AddOption(TEXT("연습"));
        ModeComboBox->AddOption(TEXT("시간제"));
        ModeComboBox->AddOption(TEXT("서바이벌"));
        ModeComboBox->SetSelectedIndex(0);
    }

    // 더미 타입 옵션
    if (DummyTypeComboBox)
    {
        DummyTypeComboBox->ClearOptions();
        DummyTypeComboBox->AddOption(TEXT("정지"));
        DummyTypeComboBox->AddOption(TEXT("순찰"));
        DummyTypeComboBox->AddOption(TEXT("랜덤"));
        DummyTypeComboBox->AddOption(TEXT("반응형"));
        DummyTypeComboBox->SetSelectedIndex(0);
    }
}

void UTrainingConsoleWidget::ApplySettingsToConsole()
{
    if (!OwningConsole)
        return;

    OwningConsole->CurrentMode = SelectedMode;
    OwningConsole->DefaultDummyType = SelectedDummyType;
    OwningConsole->DummyCount = SelectedDummyCount;
    OwningConsole->TrainingDuration = SelectedDuration;
    OwningConsole->DummyMovementSpeed = SelectedMovementSpeed;

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget] 설정 적용 완료"));
}

void UTrainingConsoleWidget::ServerRequestStartTraining_Implementation(AActor* ConsoleActor)
{
    // 서버 환경 확인
    UWorld* World = GetWorld();
    ENetMode NetMode = World ? World->GetNetMode() : NM_Standalone;
    FString NetModeStr = (NetMode == NM_Client) ? TEXT("Client") :
        (NetMode == NM_DedicatedServer) ? TEXT("Server") :
        (NetMode == NM_ListenServer) ? TEXT("ListenServer") : TEXT("Standalone");

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] 실행됨 - NetMode: %s"),
        *NetModeStr);

    if (!ConsoleActor)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] ConsoleActor가 NULL"));
        return;
    }

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] ConsoleActor: %s, HasAuthority: %s"),
        *ConsoleActor->GetName(),
        ConsoleActor->HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));

    // 서버에서 전달받은 액터 참조를 Console로 캐스팅
    ATrainingConsole* Console = Cast<ATrainingConsole>(ConsoleActor);
    if (!Console)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] Console 캐스팅 실패"));
        return;
    }

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] Console 캐스팅 성공: %s"),
        *Console->GetName());

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] PlayerController가 NULL"));
        return;
    }

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] PlayerController: %s, Pawn: %s"),
        *PC->GetName(),
        PC->GetPawn() ? *PC->GetPawn()->GetName() : TEXT("NULL"));

    // 서버에서 직접 StartTraining 호출
    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] Console->StartTraining 호출"));
    Console->StartTraining(PC);

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::ServerRequestStartTraining_Implementation] 완료"));
}

void UTrainingConsoleWidget::ServerRequestStopTraining_Implementation()
{
    if (OwningConsole)
    {
        OwningConsole->StopTraining();
    }
}

void UTrainingConsoleWidget::ServerRequestResetDummies_Implementation()
{
    if (OwningConsole)
    {
        OwningConsole->ResetDummies();
    }
}

void UTrainingConsoleWidget::OnStartButtonClicked()
{
    if (!OwningConsole)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::OnStartButtonClicked] OwningConsole이 NULL"));
        return;
    }

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::OnStartButtonClicked] 시작 버튼 클릭"));

    // 설정 적용
    ApplySettingsToConsole();

    // Character의 ServerInteractWithConsole 호출
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        ABaseCharacter* Character = Cast<ABaseCharacter>(PC->GetPawn());
        if (Character)
        {
            LOG_Item_WARNING(TEXT("[Widget] ServerInteractWithConsole 호출 - Action: 0 (시작)"));
            Character->ServerInteractWithConsole(OwningConsole, 0);
        }
        else
        {
            LOG_Item_WARNING(TEXT("[Widget] Character 캐스팅 실패"));
        }
    }
    else
    {
        LOG_Item_WARNING(TEXT("[Widget] PlayerController가 NULL"));
    }

    OnCloseButtonClicked();
}

void UTrainingConsoleWidget::OnStopButtonClicked()
{
    if (!OwningConsole)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::OnStopButtonClicked] OwningConsole이 NULL"));
        return;
    }

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::OnStopButtonClicked] 중지 버튼 클릭"));

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        ABaseCharacter* Character = Cast<ABaseCharacter>(PC->GetPawn());
        if (Character)
        {
            LOG_Item_WARNING(TEXT("[Widget] ServerInteractWithConsole 호출 - Action: 1 (중지)"));
            Character->ServerInteractWithConsole(OwningConsole, 1);
        }
    }

    UpdateButtonStates(false);
}

void UTrainingConsoleWidget::OnResetButtonClicked()
{
    if (!OwningConsole)
    {
        LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::OnResetButtonClicked] OwningConsole이 NULL"));
        return;
    }

    LOG_Item_WARNING(TEXT("[TrainingConsoleWidget::OnResetButtonClicked] 리셋 버튼 클릭"));

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        ABaseCharacter* Character = Cast<ABaseCharacter>(PC->GetPawn());
        if (Character)
        {
            LOG_Item_WARNING(TEXT("[Widget] ServerInteractWithConsole 호출 - Action: 2 (리셋)"));
            Character->ServerInteractWithConsole(OwningConsole, 2);
        }
    }
}

void UTrainingConsoleWidget::OnCloseButtonClicked()
{
    // UI 닫기
    RemoveFromParent();

    // 입력 모드를 게임 전용으로 복원
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}

void UTrainingConsoleWidget::OnModeSelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SelectedItem == TEXT("연습"))
        SelectedMode = ETrainingMode::Practice;
    else if (SelectedItem == TEXT("시간제"))
        SelectedMode = ETrainingMode::Timed;
    else if (SelectedItem == TEXT("서바이벌"))
        SelectedMode = ETrainingMode::Survival;
}

void UTrainingConsoleWidget::OnDummyTypeSelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SelectedItem == TEXT("정지"))
        SelectedDummyType = EDummyType::Static;
    else if (SelectedItem == TEXT("순찰"))
        SelectedDummyType = EDummyType::Patrol;
    else if (SelectedItem == TEXT("랜덤"))
        SelectedDummyType = EDummyType::Random;
    else if (SelectedItem == TEXT("반응형"))
        SelectedDummyType = EDummyType::Reactive;
}

void UTrainingConsoleWidget::OnDummyCountChanged(float Value)
{
    SelectedDummyCount = FMath::RoundToInt(Value);
    if (DummyCountText)
    {
        DummyCountText->SetText(FText::AsNumber(SelectedDummyCount));
    }
}

void UTrainingConsoleWidget::OnTimeChanged(float Value)
{
    SelectedDuration = Value;
    if (TimeText)
    {
        FString TimeStr = FString::Printf(TEXT("%.0f초"), SelectedDuration);
        TimeText->SetText(FText::FromString(TimeStr));
    }
}

void UTrainingConsoleWidget::OnMovementSpeedChanged(float Value)
{
    SelectedMovementSpeed = Value;
    if (MovementSpeedText)
    {
        FString SpeedStr = FString::Printf(TEXT("%.0f"), SelectedMovementSpeed);
        MovementSpeedText->SetText(FText::FromString(SpeedStr));
    }
}

