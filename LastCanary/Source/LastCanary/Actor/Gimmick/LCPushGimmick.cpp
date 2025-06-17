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
		LOG_Art_WARNING(TEXT(" Blocker 감지로 인해 이동 차단됨"));
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

	MoveVector = PushDirection * PushDistance;
	Super::StartMovement();
}

bool ALCPushGimmick::DeterminePushDirection(FVector& OutDirection)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!IsValid(PC))
	{
		LOG_Art_WARNING(TEXT(" GetOwner() 실패 - PlayerController 아님"));
		return false;
	}

	APawn* Pawn = PC->GetPawn();
	if (!IsValid(Pawn))
	{
		LOG_Art_WARNING(TEXT(" PlayerController → Pawn 변환 실패"));
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

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.5f, 0, 2.0f);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit && Hit.GetActor()->ActorHasTag(FName("GimmickBlocker")))
	{
		LOG_Art(Log, TEXT(" 라인트레이스로 Blocker 감지됨: %s"), *Hit.GetActor()->GetName());
		return true;
	}

	return false;
}

void ALCPushGimmick::StartRotation()
{
	LOG_Art(Log, TEXT("[PushGimmick] 회전 스킵"));
}

void ALCPushGimmick::StartServerRotation(const FQuat& From, const FQuat& To, float Duration)
{
	LOG_Art(Log, TEXT("[PushGimmick] StartServerRotation 스킵"));
}

void ALCPushGimmick::StartClientRotation(const FQuat& From, const FQuat& To, float Duration, bool bReturn)
{
	LOG_Art(Log, TEXT("[PushGimmick] StartClientRotation 스킵"));
}

void ALCPushGimmick::StartClientSyncRotation_Implementation(const FQuat& From, const FQuat& To, float Duration)
{
	LOG_Art(Log, TEXT("ALCPushGimmick ▶ StartClientSyncRotation - Duration: %.2f"), Duration);

	ClientStartQuat = From;
	ClientTargetQuat = To;
	ClientRotationDuration = Duration;
	ClientRotationElapsed = 0.f;

	SetActorRotation(From); 

	GetWorldTimerManager().SetTimer(
		RotationInterpTimer,
		this,
		&ALCPushGimmick::StepClientSyncRotation,
		0.02f,
		true
	);
}

void ALCPushGimmick::StepClientSyncRotation()
{
	ClientRotationElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ClientRotationElapsed / ClientRotationDuration, 0.f, 1.f);
	const FQuat InterpQuat = FQuat::Slerp(ClientStartQuat, ClientTargetQuat, Alpha);

	SetActorRotation(InterpQuat);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(RotationInterpTimer);
	}
}
