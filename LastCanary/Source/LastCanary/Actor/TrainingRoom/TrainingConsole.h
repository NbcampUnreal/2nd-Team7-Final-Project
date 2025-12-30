#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "UI/UIElement/TrainingConsoleWidget.h"
#include "DataType/TrainingType.h"
#include "TrainingConsole.generated.h"


UCLASS()
class LASTCANARY_API ATrainingConsole : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    ATrainingConsole();

    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------

    /** 콘솔 메시 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ConsoleMesh;

    //-----------------------------------------------------
    // 설정
    //-----------------------------------------------------

    /** 연결된 훈련장 매니저 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    class ATrainingRoomManager* TrainingManager;

    /** 훈련 모드 (게임 룰) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Session")
    ETrainingMode CurrentMode = ETrainingMode::Practice;

    /** 더미 스폰 개수 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training", meta = (ClampMin = "1", ClampMax = "20"))
    int32 DummyCount = 5;

    /** 훈련 시간 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training", meta = (ClampMin = "0", ClampMax = "300"))
    float TrainingDuration = 60.0f;


    UPROPERTY(BlueprintReadWrite, Replicated, Category = "Training")
    uint8 PendingAction = 255; // 255: 없음, 0: 시작, 1: 중지, 2: 리셋

    //-----------------------------------------------------
    // 더미 행동 설정
    //-----------------------------------------------------

    /** 생성할 더미의 기본 타입 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Dummy")
    EDummyType DefaultDummyType = EDummyType::Static;

    /** 이동 속도 (이동형 더미용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Dummy",
        meta = (EditCondition = "DefaultDummyType != EDummyType::Static", ClampMin = "0.0", ClampMax = "1000.0"))
    float DummyMovementSpeed = 100.0f;

    /** 랜덤 이동 반경 (Random 타입용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training|Dummy",
        meta = (EditCondition = "DefaultDummyType == EDummyType::Random", ClampMin = "0.0", ClampMax = "5000.0"))
    float RandomMovementRadius = 500.0f;

    //-----------------------------------------------------
    // IInteractableInterface 구현
    //-----------------------------------------------------

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(APlayerController* Interactor);
    virtual void Interact_Implementation(APlayerController* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FString GetInteractMessage() const;
    virtual FString GetInteractMessage_Implementation() const;

    //-----------------------------------------------------
    // 상호작용 함수
    //-----------------------------------------------------

    /** 훈련 시작 버튼 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void StartTraining(APlayerController* Interactor);

    UFUNCTION(Server, Reliable)
    void ServerStartTraining(APlayerController* Interactor);
    void ServerStartTraining_Implementation(APlayerController* Interactor);

    /** 훈련 종료 버튼 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void StopTraining();

    /** 더미 리셋 버튼 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void ResetDummies();

    /** 설정 적용 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void ApplySettings();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 대기 중인 동작 실행 */
    void ExecutePendingAction(APlayerController* Interactor);

private:
    /** 매니저 자동 찾기 */
    void FindTrainingManager();

    //-----------------------------------------------------
    // 콘솔 UI
    //-----------------------------------------------------

private:
    UPROPERTY(EditAnywhere, Category = "WidgetClasses")
    TSubclassOf<UTrainingConsoleWidget> ConsoleWidgetClass;
};
