#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "LCSplineGimmick.generated.h"

class USplineComponent;

UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCSplineGimmick : public ALCBaseGimmick
{
    GENERATED_BODY()

public:
    ALCSplineGimmick();

protected:
    virtual void BeginPlay() override;

public:
    /** Spline 컴포넌트 */
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Spline")
    AActor* SplineActorRef;

    USplineComponent* TargetSpline;

    /** 현재 Spline 상 거리(0~SplineLength) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    float CurrentDistance;

    /** 한 번 상호작용 시 이동 거리(단위: cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
    float StepDistance;

    /** 최대 이동 속도(감속 전) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
    float MaxSpeed;

    /** 감속(브레이크) 시작 거리(레일 끝과의 거리) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
    float BrakeDistance;

    /** 이동 중 여부 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    bool bIsMoving;

    /** 최종 목표 거리(Stop 시점) */
    float TargetDistance;

    /** 이동 타이머 */
    FTimerHandle MoveTimerHandle;

    /** 분기/장애물 등으로 이동 차단 시 true */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    bool bBlocked;

    /** 상호작용(플레이어/트리거)로 이동 시작 */
    virtual void Interact_Implementation(APlayerController* Interactor) override;

    /** 실제 이동 로직 */
    void StartMove(int Direction);

    /** 이동 step마다 호출(보간, 감속) */
    void StepMove();

    /** 이동 멈춤/정지 */
    void StopMove();

    /** 레일 끝/분기/장애물 체크 및 정지 */
    bool CheckForEndOrBlocked(float NextDistance);

    /** 분기점/장애물/연결 레일 감지(확장 가능) */
    bool IsBranchBlocked(float SplineDistance);

    /** 현재 이동 방향(1=정방향, -1=역방향) */
    int MoveDirection;

    /** 미는 위치/방향 판정(플레이어/트리거 기준) */
    int GetInteractionDirection(APlayerController* Interactor) const;

    /** 이동 완료 델리게이트 이벤트 */
    //DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveStopped);
    //UPROPERTY(BlueprintAssignable, Category="Spline")
    //FOnMoveStopped OnMoveStopped;
};
