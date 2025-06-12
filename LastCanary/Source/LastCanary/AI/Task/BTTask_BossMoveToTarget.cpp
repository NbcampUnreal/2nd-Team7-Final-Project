#include "AI/Task/BTTask_BossMoveToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_BossMoveToTarget::UBTTask_BossMoveToTarget(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    NodeName = TEXT("Boss Move To Target");
    bNotifyTick = true;          // ExecuteTask 후 TickTask 호출
    bCreateNodeInstance = true;  // 노드별 인스턴스 유지

    // 기본값 설정(필요 시 블루프린트에서 조정 가능)
    TeleportDistance = 1500.f;
    JumpHeightThreshold = 100.f;
    JumpForwardStrength = 600.f;
    JumpUpStrength = 800.f;
    MyAcceptableRadius = 100.f;
}

EBTNodeResult::Type UBTTask_BossMoveToTarget::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
    if (!TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    // 처음에 한 번 이동 명령
    AIController->MoveToActor(TargetActor, MyAcceptableRadius);

    return EBTNodeResult::InProgress;
}

void UBTTask_BossMoveToTarget::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BBComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject("TargetActor"));
    if (!TargetActor)
    {
        AIController->StopMovement();
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 1) 현재 위치, 목표 위치, 3D 거리 계산
    const FVector CurrentLoc = ControlledPawn->GetActorLocation();
    const FVector TargetLoc = TargetActor->GetActorLocation();
    const float DistanceToTarget = FVector::Dist(CurrentLoc, TargetLoc);

    // 2) 목표가 너무 멀어지면 텔레포트
    if (DistanceToTarget > TeleportDistance)
    {
        FVector Direction = (CurrentLoc - TargetLoc).GetSafeNormal();
        if (Direction.IsZero())
        {
            Direction = FVector::ForwardVector;
        }
        FVector TeleportPos = TargetLoc + Direction * (MyAcceptableRadius + 10.f);

        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
        if (NavSys)
        {
            FNavLocation ProjectedLocation;
            if (NavSys->ProjectPointToNavigation(
                TeleportPos, ProjectedLocation, FVector(500.f, 500.f, 500.f)))
            {
                TeleportPos = ProjectedLocation.Location;
            }
        }

        ControlledPawn->SetActorLocation(TeleportPos);
        AIController->StopMovement();
        AIController->MoveToActor(TargetActor, MyAcceptableRadius);
        return;
    }

    // 3) “NavMesh 상의 높이”를 구해서 높이 차이 계산
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
    float   BossNavZ = CurrentLoc.Z;
    float   TargetNavZ = TargetLoc.Z;

    if (NavSys)
    {
        FNavLocation BossNavLocation, TargetNavLocation;

        // (a) 보스의 XY 좌표를 NavMesh에 투영해서 실제 지형(바닥) 높이 얻기
        if (NavSys->ProjectPointToNavigation(
            CurrentLoc, BossNavLocation, FVector(50.f, 50.f, 200.f)))
        {
            BossNavZ = BossNavLocation.Location.Z;
        }

        // (b) 타겟의 XY 좌표를 NavMesh에 투영해서 실제 지형(바닥) 높이 얻기
        if (NavSys->ProjectPointToNavigation(
            TargetLoc, TargetNavLocation, FVector(50.f, 50.f, 200.f)))
        {
            TargetNavZ = TargetNavLocation.Location.Z;
        }
    }

    const float ZDiffOnNav = TargetNavZ - BossNavZ;

    // 4) NavMesh 상 높이 차이가 임계값 이상이면, 앞으로+위로 점프(LaunchCharacter) 처리
    if (FMath::Abs(ZDiffOnNav) > JumpHeightThreshold)
    {
        ACharacter* BossChar = Cast<ACharacter>(ControlledPawn);
        if (BossChar)
        {
            UCharacterMovementComponent* MoveComp = BossChar->GetCharacterMovement();
            if (MoveComp && MoveComp->IsMovingOnGround())
            {
                // XY 평면에서 목표를 향하는 방향 계산
                FVector DirXY = (TargetLoc - CurrentLoc).GetSafeNormal2D();
                if (DirXY.IsZero())
                {
                    DirXY = BossChar->GetActorForwardVector().GetSafeNormal2D();
                }

                // LaunchCharacter에 넣을 속도 벡터: 
                //   수평 성분 = DirXY * JumpForwardStrength
                //   수직 성분 = JumpUpStrength
                FVector LaunchVelocity = FVector(
                    DirXY.X * JumpForwardStrength,
                    DirXY.Y * JumpForwardStrength,
                    JumpUpStrength
                );

                BossChar->LaunchCharacter(LaunchVelocity, true, true);

                // 점프 직후 AI 경로 끊었다가 다시 MoveToActor
                AIController->StopMovement();
                AIController->MoveToActor(TargetActor, MyAcceptableRadius);
            }
        }
        return;
    }

    // 5) 정상적인 NavMesh 이동: Idle 상태일 때 재경로
    EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        AIController->MoveToActor(TargetActor, MyAcceptableRadius);
    }

    // 6) 목표 도달 여부 확인
    if (DistanceToTarget <= MyAcceptableRadius + 50.f)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
