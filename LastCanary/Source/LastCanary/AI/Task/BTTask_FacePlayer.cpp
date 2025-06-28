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
    if (AAIController* AICon = OwnerComp.GetAIOwner())
    {
        // 진행 중이던 Patrol MoveTo 요청을 즉시 중지
        AICon->StopMovement();
    }

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
    FRotator DesiredRot = ToTarget.Rotation();
    FRotator CurrentRot = Pawn->GetActorRotation();
    float YawDiff = FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, DesiredRot.Yaw);

    // 2) 데드존 내에 있으면 성공
    if (FMath::Abs(YawDiff) <= DeadzoneAngle)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 3) 부드러운 회전 (RInterpConstantTo)
    float YawStep = FMath::Sign(YawDiff) * RotateSpeed * DeltaSeconds;
    // 만약 한 스텝이 YawDiff보다 크면, 딱 맞추도록 클램프
    if (FMath::Abs(YawStep) > FMath::Abs(YawDiff))
    {
        YawStep = YawDiff;
    }
    FRotator NewRot = CurrentRot;
    NewRot.Yaw += YawStep;
    Pawn->SetActorRotation(NewRot);
}