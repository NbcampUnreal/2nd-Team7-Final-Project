#include "Actor/Gimmick/LCPushGimmick.h"
#include "Components/BoxComponent.h"
#include "Character/BasePlayerController.h"
#include "LastCanary.h"

ALCPushGimmick::ALCPushGimmick()
	: PushDistance(100.f)
	, bBlockedByWall(false)
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
		if (IsValid(Interactor) && GetOwner() != Interactor)
		{
			SetOwner(Interactor);
		}
	}
	Super::Interact_Implementation(Interactor);
}

void ALCPushGimmick::ActivateGimmick_Implementation()
{
	if (!HasAuthority())
	{
		Server_ActivateGimmick();
		return;
	}

	if (!ILCGimmickInterface::Execute_CanActivate(this))
	{
		return;
	}

	FVector PushDirection;
	if (!DeterminePushDirection(PushDirection))
	{
		return;
	}

	bBlockedByWall = IsBlockedByWall(PushDirection);
	if (bBlockedByWall)
	{
		LOG_Art_WARNING(TEXT("▶ Blocker 감지로 인해 이동 차단됨"));
		return;
	}

	MoveVector = PushDirection * PushDistance;
	Super::ActivateGimmick_Implementation();
}


void ALCPushGimmick::StartMovement()
{
	if (bIsMovingServer || bIsReturningServer)
	{
		return;
	}

	FVector PushDirection;

	if (!DeterminePushDirection(PushDirection))
	{
		return;
	}

	MoveVector = PushDirection * 600.f;
	Super::StartMovement();
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
		return true;
	}

	if (BackwardTrigger && BackwardTrigger->IsOverlappingActor(Pawn))
	{
		OutDirection = Forward;
		return true;
	}
	return false;
}

bool ALCPushGimmick::CanActivate_Implementation()
{
	if (bIsMovingServer || bIsReturningServer)
	{
		return false;
	}
	return Super::CanActivate_Implementation();
}

bool ALCPushGimmick::IsBlockedByWall(const FVector& Direction)
{
	FHitResult Hit;
	const FVector Start = GetActorLocation();
	const FVector End = Start + Direction * 500.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit && Hit.GetActor()->ActorHasTag(FName("GimmickBlocker")))
	{
		LOG_Art(Log, TEXT("▶ 라인트레이스로 Blocker 감지됨: %s"), *Hit.GetActor()->GetName());
		return true;
	}

	return false;
}
