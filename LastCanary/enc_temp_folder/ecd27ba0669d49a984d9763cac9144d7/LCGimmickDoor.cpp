#include "Actor/Gimmick/Effect/LCGimmickDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

ALCGimmickDoor::ALCGimmickDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;

	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
}

void ALCGimmickDoor::BeginPlay()
{
	Super::BeginPlay();

	if (OpenCurve)
	{
		FOnTimelineFloat Progress;
		Progress.BindUFunction(this, FName("OnDoorTimelineUpdate"));
		DoorTimeline->AddInterpFloat(OpenCurve, Progress);

		FOnTimelineEvent Finished;
		Finished.BindUFunction(this, FName("OnDoorTimelineFinished"));
		DoorTimeline->SetTimelineFinishedFunc(Finished);
	}

	InitialLocation = GetActorLocation();
	InitialRotation = GetActorRotation();

	ClosedLocation = GetActorLocation();
	ClosedRotation = GetActorRotation();

	OpenedLocation = ClosedLocation + SlideOffset;
	OpenedRotation = ClosedRotation + OpenRotation;
}

void ALCGimmickDoor::OnDoorTimelineUpdate(float Value)
{
	if (bSlideInstead)
	{
		FVector NewLocation = FMath::Lerp(InitialLocation, TargetLocation, Value);
		SetActorLocation(NewLocation);
	}
	else
	{
		FRotator NewRotation = FMath::Lerp(InitialRotation, TargetRotation, Value);
		SetActorRotation(NewRotation);
	}
}

void ALCGimmickDoor::OnDoorTimelineFinished()
{
	LOG_Frame_WARNING(TEXT("[GimmickDoor] Animation completed."));
}

void ALCGimmickDoor::StartDoorAnimation(bool bOpen)
{
	if (!OpenCurve)
	{
		return;
	}

	if (DoorTimeline->IsPlaying())
	{
		DoorTimeline->Stop();
	}

	this->bIsOpen = bOpen;

	if (bSlideInstead)
	{
		InitialLocation = GetActorLocation(); // 현재 위치에서 시작
		TargetLocation = bOpen ? OpenedLocation : ClosedLocation;
	}
	else
	{
		InitialRotation = GetActorRotation(); // 현재 회전에서 시작
		TargetRotation = bOpen ? OpenedRotation : ClosedRotation;
	}

	DoorTimeline->PlayFromStart();
}

void ALCGimmickDoor::TriggerEffect_Implementation()
{
	if (bIsOpen)
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("[GimmickDoor] TriggerEffect → Open"));
	StartDoorAnimation(true);
}

void ALCGimmickDoor::StopEffect_Implementation()
{
	if (!bIsOpen)
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("[GimmickDoor] StopEffect → Close"));
	StartDoorAnimation(false);
}
