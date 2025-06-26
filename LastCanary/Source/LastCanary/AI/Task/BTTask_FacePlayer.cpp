#include "AI/Task/BTTask_FacePlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_FacePlayer::UBTTask_FacePlayer()
{
    NodeName = TEXT("Face Player");
    bNotifyTick = true;  // TickTask를 호출하도록
}

EBTNodeResult::Type UBTTask_FacePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 바로 InProgress로 리턴하면 TickTask가 호출됩니다
    return EBTNodeResult::InProgress;
}

void UBTTask_FacePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    UObject* Obj = BB->GetValueAsObject(TEXT("TargetActor"));
    AActor* Target = Cast<AActor>(Obj);
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Target || !Pawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 1) 방향 계산
    FVector ToTarget = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal2D();
    FRotator Desired = ToTarget.Rotation();
    FRotator Current = Pawn->GetActorRotation();
    float YawDiff = FMath::FindDeltaAngleDegrees(Current.Yaw, Desired.Yaw);

    // 2) 단계별 회전
    float MaxStep = RotateSpeed * DeltaSeconds;
    float YawStep = FMath::Clamp(YawDiff, -MaxStep, +MaxStep);
    Pawn->SetActorRotation(Current + FRotator(0, YawStep, 0));

    // 3) 완료 조건
    if (FMath::Abs(YawDiff) <= AcceptanceAngle)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}