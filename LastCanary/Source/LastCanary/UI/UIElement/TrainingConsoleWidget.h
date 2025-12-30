#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/TrainingType.h"
#include "TrainingConsoleWidget.generated.h"

class UButton;
class UComboBoxString;
class USlider;
class UTextBlock;
class ATrainingConsole;

UCLASS()
class LASTCANARY_API UTrainingConsoleWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
    //-----------------------------------------------------
    // UI 컴포넌트 바인딩
    //-----------------------------------------------------

    /** 훈련 시작 버튼 */
    UPROPERTY(meta = (BindWidget))
    UButton* StartButton;

    /** 훈련 중지 버튼 */
    UPROPERTY(meta = (BindWidget))
    UButton* StopButton;

    /** 리셋 버튼 */
    UPROPERTY(meta = (BindWidget))
    UButton* ResetButton;

    /** 닫기 버튼 */
    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    /** 훈련 모드 선택 드롭다운 */
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* ModeComboBox;

    /** 더미 타입 선택 드롭다운 */
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* DummyTypeComboBox;

    /** 더미 개수 슬라이더 */
    UPROPERTY(meta = (BindWidget))
    USlider* DummyCountSlider;

    /** 더미 개수 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DummyCountText;

    /** 훈련 시간 슬라이더 */
    UPROPERTY(meta = (BindWidget))
    USlider* TimeSlider;

    /** 훈련 시간 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TimeText;

    /** 이동 속도 슬라이더 */
    UPROPERTY(meta = (BindWidget))
    USlider* MovementSpeedSlider;

    /** 이동 속도 텍스트 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* MovementSpeedText;

    //-----------------------------------------------------
    // 설정 값
    //-----------------------------------------------------

    /** 연결된 콘솔 액터 */
    UPROPERTY(BlueprintReadOnly, Category = "Training")
    ATrainingConsole* OwningConsole;

    /** 선택된 훈련 모드 */
    UPROPERTY(BlueprintReadWrite, Category = "Training")
    ETrainingMode SelectedMode;

    /** 선택된 더미 타입 */
    UPROPERTY(BlueprintReadWrite, Category = "Training")
    EDummyType SelectedDummyType;

    /** 선택된 더미 개수 */
    UPROPERTY(BlueprintReadWrite, Category = "Training")
    int32 SelectedDummyCount;

    /** 선택된 훈련 시간 */
    UPROPERTY(BlueprintReadWrite, Category = "Training")
    float SelectedDuration;

    /** 선택된 이동 속도 */
    UPROPERTY(BlueprintReadWrite, Category = "Training")
    float SelectedMovementSpeed;

    //-----------------------------------------------------
    // 함수
    //-----------------------------------------------------

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

public:
    /** 콘솔 참조 설정 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void SetOwningConsole(ATrainingConsole* Console);

    /** UI 초기화 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void InitializeUI();

    /** 훈련 상태에 따라 버튼 활성화/비활성화 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void UpdateButtonStates(bool bTrainingActive);

private:
    //-----------------------------------------------------
    // 버튼 이벤트
    //-----------------------------------------------------

    UFUNCTION()
    void OnStartButtonClicked();

    UFUNCTION()
    void OnStopButtonClicked();

    UFUNCTION()
    void OnResetButtonClicked();

    UFUNCTION()
    void OnCloseButtonClicked();

    //-----------------------------------------------------
    // 드롭다운 이벤트
    //-----------------------------------------------------

    UFUNCTION()
    void OnModeSelected(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnDummyTypeSelected(FString SelectedItem, ESelectInfo::Type SelectionType);

    //-----------------------------------------------------
    // 슬라이더 이벤트
    //-----------------------------------------------------

    UFUNCTION()
    void OnDummyCountChanged(float Value);

    UFUNCTION()
    void OnTimeChanged(float Value);

    UFUNCTION()
    void OnMovementSpeedChanged(float Value);

    //-----------------------------------------------------
    // 헬퍼 함수
    //-----------------------------------------------------

    /** 콤보박스 옵션 채우기 */
    void PopulateComboBoxes();

    /** 현재 설정값을 콘솔에 적용 */
    void ApplySettingsToConsole();

    //-----------------------------------------------------
    // 서버 함수
    //-----------------------------------------------------

    /** 서버에서 훈련 시작 요청 (Widget이 소유되어 있으므로 RPC 가능) */
    UFUNCTION(Server, Reliable)
    void ServerRequestStartTraining(AActor* ConsoleActor);
    void ServerRequestStartTraining_Implementation(AActor* ConsoleActor);

    /** 서버에서 훈련 중지 요청 */
    UFUNCTION(Server, Reliable)
    void ServerRequestStopTraining();
    void ServerRequestStopTraining_Implementation();

    /** 서버에서 더미 리셋 요청 */
    UFUNCTION(Server, Reliable)
    void ServerRequestResetDummies();
    void ServerRequestResetDummies_Implementation();
};
