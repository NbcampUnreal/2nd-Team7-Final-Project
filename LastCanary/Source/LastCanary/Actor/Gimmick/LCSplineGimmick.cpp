#include "Actor/Gimmick/LCSplineGimmick.h"
#include "TimerManager.h"
#include "Components/SplineComponent.h"
#include "GameFramework/PlayerController.h"
#include "LastCanary.h"

ALCSplineGimmick::ALCSplineGimmick()
    : TargetSpline(nullptr)
    , CurrentDistance(0.f)
    , StepDistance(1000.f)
    , MaxSpeed(1000.f)
    , BrakeDistance(300.f)
    , bIsMoving(false)
    , TargetDistance(0.f)
    , bBlocked(false)
    , MoveDirection(1)
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
}

void ALCSplineGimmick::BeginPlay()
{
    Super::BeginPlay();

    if (SplineActorRef)
    {
        TargetSpline = SplineActorRef->FindComponentByClass<USplineComponent>();
        LOG_Art(Log, TEXT("[BeginPlay] SplineActorRef: %s | TargetSpline: %s"),
            *GetNameSafe(SplineActorRef), *GetNameSafe(TargetSpline));
    }
    else
    {
        LOG_Art_ERROR(TEXT("[BeginPlay] SplineActorRef가 할당되지 않았음!"));
    }

    if (TargetSpline)
    {
        float ClosestKey = TargetSpline->FindInputKeyClosestToWorldLocation(GetActorLocation());
        CurrentDistance = TargetSpline->GetDistanceAlongSplineAtSplineInputKey(ClosestKey);

        LOG_Art(Log, TEXT("[BeginPlay] 현재 카트 위치 기준 Spline Distance 초기화: %.1f (ClosestKey: %.2f)"), CurrentDistance, ClosestKey);

        // Spline 위로 정확히 맞추려면 위치/회전도 Spline상 위치로 맞춰줌(초기화)
        FVector NewLoc = TargetSpline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        FRotator NewRot = TargetSpline->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        SetActorLocationAndRotation(NewLoc, NewRot);
    }

    LOG_Art(Log, TEXT("[BeginPlay] CurrentDistance: %.1f | StepDistance: %.1f"), CurrentDistance, StepDistance);
}

void ALCSplineGimmick::Interact_Implementation(APlayerController* Interactor)
{
    if (!HasAuthority() || bIsMoving || !TargetSpline)
    {
        LOG_Art_WARNING(TEXT("[Interact] 조건 불충족: Authority: %d | bIsMoving: %d | TargetSpline: %s"),
            HasAuthority(), bIsMoving, *GetNameSafe(TargetSpline));
        return;
    }

    int Dir = GetInteractionDirection(Interactor);
    LOG_Art(Log, TEXT("[Interact] 상호작용자: %s | 방향: %d"), *GetNameSafe(Interactor), Dir);

    if (Dir == 0)
    {
        LOG_Art_WARNING(TEXT("[Interact] 이동 방향 판별 실패"));
        return;
    }

    StartMove(Dir);
}

int ALCSplineGimmick::GetInteractionDirection(APlayerController* Interactor) const
{
    // 기본: 플레이어 위치 기준, 앞쪽이면 -1(뒤로), 뒤쪽이면 +1(앞으로)
    if (!Interactor || !TargetSpline) return 1;

    APawn* Pawn = Interactor->GetPawn();
    if (!Pawn) return 1;

    FVector CartLoc = GetActorLocation();
    FVector SplineTangent = TargetSpline->GetTangentAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
    FVector PlayerLoc = Pawn->GetActorLocation();
    FVector ToPlayer = (PlayerLoc - CartLoc).GetSafeNormal();
    float Dot = FVector::DotProduct(SplineTangent, ToPlayer);

    return (Dot > 0) ? -1 : 1;
}

void ALCSplineGimmick::StartMove(int Direction)
{
    if (!TargetSpline)
    {
        return;
    }

    MoveDirection = Direction;
    bIsMoving = true;
    bBlocked = false;

    float SplineLength = TargetSpline->GetSplineLength();
    TargetDistance = FMath::Clamp(CurrentDistance + StepDistance * MoveDirection, 0.f, SplineLength);

    LOG_Art(Log, TEXT("[StartMove] 이동 시작 | 방향: %d | 현재: %.1f | 목표: %.1f | SplineLength: %.1f"),
        MoveDirection, CurrentDistance, TargetDistance, SplineLength);

    GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &ALCSplineGimmick::StepMove, 0.02f, true);
}

void ALCSplineGimmick::StepMove()
{
    if (!TargetSpline || !bIsMoving)
    {
        LOG_Art_WARNING(TEXT("[StepMove] 이동 도중 예외 발생! bIsMoving: %d | TargetSpline: %s"),
            bIsMoving, *GetNameSafe(TargetSpline));
        StopMove();
        return;
    }

    float DeltaTime = 0.02f;
    float SplineLength = TargetSpline->GetSplineLength();

    // 감속 비율
    float DistanceToTarget = FMath::Abs(TargetDistance - CurrentDistance);
    float DistanceToEnd = (MoveDirection > 0) ? SplineLength - CurrentDistance : CurrentDistance;
    float BrakeAlpha = 1.f;

    float Speed = MaxSpeed * BrakeAlpha;
    float MoveStep = Speed * DeltaTime * MoveDirection;

    LOG_Art(Log, TEXT("[StepMove] Cur: %.1f | Target: %.1f | Speed: %.1f | BrakeAlpha: %.2f | MoveStep: %.1f"),
        CurrentDistance, TargetDistance, Speed, BrakeAlpha, MoveStep);

    if (DistanceToEnd < BrakeDistance)
    {
        BrakeAlpha = DistanceToEnd / BrakeDistance;
    }

    bool bArrived = false;

    if (MoveDirection > 0)
    {
        if (CurrentDistance + MoveStep >= TargetDistance)
        {
            CurrentDistance = TargetDistance;
            bArrived = true;
        }
        else
        {
            CurrentDistance += MoveStep;
        }
    }
    else
    {
        if (CurrentDistance + MoveStep <= TargetDistance)
        {
            CurrentDistance = TargetDistance;
            bArrived = true;
        }
        else
        {
            CurrentDistance += MoveStep;
        }
    }

    FVector NewLoc = TargetSpline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
    FRotator NewRot = TargetSpline->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
    SetActorLocationAndRotation(NewLoc, NewRot);

    LOG_Art(Log, TEXT("[StepMove] 이동 위치: %s | 회전: %s"),
        *NewLoc.ToCompactString(), *NewRot.ToCompactString());

    if (CheckForEndOrBlocked(CurrentDistance))
    {
        StopMove();
        return;
    }

    if (bArrived)
    {
        StopMove();
    }
}

void ALCSplineGimmick::StopMove()
{
    bIsMoving = false;
    GetWorldTimerManager().ClearTimer(MoveTimerHandle);
    LOG_Art(Log, TEXT("[StopMove] 이동 정지! 최종 위치: %.1f | Blocked: %d"), CurrentDistance, bBlocked);

    // 필요시 이동 완료 이벤트/사운드 등 호출
}

bool ALCSplineGimmick::CheckForEndOrBlocked(float NextDistance)
{
    if (!TargetSpline) return true;

    float SplineLength = TargetSpline->GetSplineLength();
    if (NextDistance <= 0.f || NextDistance >= SplineLength)
    {
        LOG_Art_WARNING(TEXT("[CheckForEndOrBlocked] Spline 끝에 도달!"));
        return true;
    }

    // 2. 분기점/TransformGimmick/장애물 등 커스텀 감지(확장 가능)
    if (IsBranchBlocked(NextDistance))
    {
        LOG_Art_WARNING(TEXT("[CheckForEndOrBlocked] 분기/장애물 차단! NextDistance: %.1f"), NextDistance);
        bBlocked = true;
        return true;
    }

    return false;
}

bool ALCSplineGimmick::IsBranchBlocked(float SplineDistance)
{
    // 확장 포인트: Spline 구간별 장애물/분기점 등 체크
    // 예: Spline Distance에 TransformGimmick이 있고, 연결/회전X면 true
    return false;
}
