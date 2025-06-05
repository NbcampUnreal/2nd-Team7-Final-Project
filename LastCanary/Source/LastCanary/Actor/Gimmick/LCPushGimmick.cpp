#include "Actor/Gimmick/LCPushGimmick.h"
#include "Components/BoxComponent.h"
#include "Character/BasePlayerController.h"
#include "LastCanary.h"

ALCPushGimmick::ALCPushGimmick()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	ForwardTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ForwardTrigger"));
	ForwardTrigger->SetupAttachment(VisualMesh);
	ForwardTrigger->SetRelativeLocation(FVector(100.f, 0.f, 0.f)); 
	ForwardTrigger->SetBoxExtent(FVector(30.f, 60.f, 60.f));
	ForwardTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ForwardTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	ForwardTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	BackwardTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BackwardTrigger"));
	BackwardTrigger->SetupAttachment(VisualMesh);
	BackwardTrigger->SetRelativeLocation(FVector(-100.f, 0.f, 0.f)); 
	BackwardTrigger->SetBoxExtent(FVector(30.f, 60.f, 60.f));
	BackwardTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BackwardTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	BackwardTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ALCPushGimmick::BeginPlay()
{
	Super::BeginPlay();

}

void ALCPushGimmick::Interact_Implementation(APlayerController* Interactor)
{
	if (HasAuthority())
	{
		APlayerController* PC = Cast<APlayerController>(Interactor);
		if (PC && GetOwner() != PC)
		{
			SetOwner(PC); 
		}
	}

	Super::Interact_Implementation(Interactor); 
}

void ALCPushGimmick::StartMovement()
{
	if (bIsMovingServer || bIsReturningServer)
	{
		LOG_Art_WARNING(TEXT("▶ StartMovement - 이동 중으로 무시됨"));
		return;
	}

	FVector PushDirection;
	if (!DeterminePushDirection(PushDirection))
	{
		LOG_Art_WARNING(TEXT("▶ StartMovement 실패 - DeterminePushDirection 실패"));
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector Target = CurrentLocation + PushDirection * MoveStep;

	LOG_Art(Log, TEXT("▶ StartMovement ▶ 현재 위치: %s"), *CurrentLocation.ToString());
	LOG_Art(Log, TEXT("▶ StartMovement ▶ 밀기 방향: %s"), *PushDirection.ToString());
	LOG_Art(Log, TEXT("▶ StartMovement ▶ 목표 위치: %s"), *Target.ToString());

	StartMovementToTarget(Target);
}

void ALCPushGimmick::ActivateGimmick_Implementation()
{
	LOG_Art(Log, TEXT("▶ ActivateGimmick_Implementation 호출됨"));

	if (!HasAuthority())
	{
		LOG_Art(Log, TEXT("▶ 클라이언트이므로 Server_ActivateGimmick 호출"));
		Server_ActivateGimmick();
		return;
	}

	if (!ILCGimmickInterface::Execute_CanActivate(this))
	{
		LOG_Art_WARNING(TEXT("▶ 쿨타임 미충족 또는 이미 활성화 상태"));
		return;
	}

	FVector PushDirection;
	if (!DeterminePushDirection(PushDirection))
	{
		LOG_Art_WARNING(TEXT("▶ DeterminePushDirection 실패 - 트리거에 캐릭터 없음"));
		return;
	}

	LOG_Art(Log, TEXT("▶ 밀기 방향: %s"), *PushDirection.ToString());

	Super::ActivateGimmick_Implementation();

	const FVector CurrentLoc = GetActorLocation();
	const FVector TargetLoc = CurrentLoc + PushDirection * MoveStep;

	LOG_Art(Log, TEXT("▶ 현재 위치: %s"), *CurrentLoc.ToString());
	LOG_Art(Log, TEXT("▶ 목표 위치: %s"), *TargetLoc.ToString());

	bUseAxis = false;
	MoveVector = FVector::ZeroVector;

	StartMovementToTarget(TargetLoc);
}

bool ALCPushGimmick::DeterminePushDirection(FVector& OutDirection)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!IsValid(PC))
	{
		LOG_Art_WARNING(TEXT("▶ GetOwner() 실패 - PlayerController 아님"));
		return false;
	}

	APawn* Pawn = PC->GetPawn();
	if (!IsValid(Pawn))
	{
		LOG_Art_WARNING(TEXT("▶ PlayerController → Pawn 변환 실패"));
		return false;
	}

	const FVector Forward = GetActorForwardVector();

	if (ForwardTrigger && ForwardTrigger->IsOverlappingActor(Pawn))
	{
		OutDirection = -Forward;
		LOG_Art(Log, TEXT("▶ ForwardTrigger 감지 → 방향: %s"), *OutDirection.ToString());
		return true;
	}

	if (BackwardTrigger && BackwardTrigger->IsOverlappingActor(Pawn))
	{
		OutDirection = Forward;
		LOG_Art(Log, TEXT("▶ BackwardTrigger 감지 → 방향: %s"), *OutDirection.ToString());
		return true;
	}

	LOG_Art(Log, TEXT("▶ 어떤 트리거도 Pawn과 Overlap 중이 아님"));
	return false;
}

bool ALCPushGimmick::CanActivate_Implementation()
{
	if (bIsMovingServer || bIsReturningServer)
	{
		LOG_Art_WARNING(TEXT("PushGimmick ▶ 이동 중이라 CanActivate 거부됨"));
		return false;
	}

	return Super::CanActivate_Implementation();
}
