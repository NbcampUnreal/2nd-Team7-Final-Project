#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataType/TrainingType.h"
#include "TrainingBoard.generated.h"

class UTrainingBoardWidget;

UCLASS()
class LASTCANARY_API ATrainingBoard : public AActor
{
    GENERATED_BODY()

public:
    ATrainingBoard();

    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------

    /** 전광판 메시 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BoardMesh;

    /** UI 위젯 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UWidgetComponent* StatsWidget;

    //-----------------------------------------------------
    // 설정
    //-----------------------------------------------------

    /** 연결된 훈련장 매니저 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    class ATrainingRoomManager* TrainingManager;

    /** 통계 UI 위젯 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    TSubclassOf<class UUserWidget> StatsWidgetClass;

    /** 자동 업데이트 간격 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float UpdateInterval = 0.5f;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** 통계 업데이트 이벤트 처리 */
    UFUNCTION()
    void OnStatsUpdated(const FTrainingStats& Stats);

    /** 세션 상태 변경 이벤트 처리 */
    UFUNCTION()
    void OnSessionStateChanged(bool bActive);

    /** UI 위젯 업데이트 (블루프린트에서 구현) */
    UFUNCTION(BlueprintImplementableEvent, Category = "Training")
    void UpdateStatsDisplay(const FTrainingStats& Stats);

    /** 세션 시작 UI 업데이트 (블루프린트에서 구현) */
    UFUNCTION(BlueprintImplementableEvent, Category = "Training")
    void OnSessionStarted();

    /** 세션 종료 UI 업데이트 (블루프린트에서 구현) */
    UFUNCTION(BlueprintImplementableEvent, Category = "Training")
    void OnSessionEnded(const FTrainingStats& FinalStats);

private:
    /** 위젯 인스턴스 참조 */
    UPROPERTY()
    UTrainingBoardWidget* BoardWidgetInstance;

    /** 경과 시간 계산용 */
    float ElapsedTime;

    /** 매니저 자동 찾기 */
    void FindTrainingManager();

    /** 업데이트 타이머 */
    FTimerHandle UpdateTimerHandle;

    /** 주기적 업데이트 */
    UFUNCTION()
    void PeriodicUpdate();
};

