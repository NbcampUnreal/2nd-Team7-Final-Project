#include "Actor/Gimmick/Trigger/LCWeightPlate.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h" // 실제 무게를 계산할 때 필요
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Interface/GimmickEffectInterface.h"

#include "LastCanary.h"

ALCWeightPlate::ALCWeightPlate()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;
	TriggerVolume->SetBoxExtent(FVector(100.f));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerVolume->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	TriggerVolume->SetGenerateOverlapEvents(true);
}

void ALCWeightPlate::BeginPlay()
{
	Super::BeginPlay();
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ALCWeightPlate::OnBeginOverlap);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ALCWeightPlate::OnEndOverlap);

	InitialLocation = GetActorLocation();
}

void ALCWeightPlate::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority()==false)
	{
		return;
	}
	if (IsValid(OtherActor)==false)
	{
		return;
	}

	OverlappingActors.AddUnique(OtherActor);
	CheckWeight();
}

void ALCWeightPlate::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (IsValid(OtherActor) == false)
	{
		return;
	}

	OverlappingActors.Remove(OtherActor);
	CheckWeight();
}

void ALCWeightPlate::CheckWeight()
{
	float TotalWeight = 0.0f;
	for (AActor* Actor : OverlappingActors)
	{
		if (IsValid(Actor))
		{
			TotalWeight += GetActorWeight(Actor);
		}
	}

	LOG_Frame_WARNING(TEXT("[WeightPlate] Current Weight: %.1f / %.1f"), TotalWeight, RequiredWeight);

	// 무게에 따른 높이 변경
	float WeightRatio = FMath::Clamp(TotalWeight / MaxWeight, 0.f, 1.f);
	TargetLocation = InitialLocation;
	//TargetLocation.Z = InitialLocation.Z + (WeightRatio * MaxOffsetZ);
	TargetLocation.X = InitialLocation.X + (WeightRatio * MaxOffsetZ);

	StartLocation = GetActorLocation();
	MoveElapsed = 0.f;

	// 이동 타이머 시작
	GetWorld()->GetTimerManager().ClearTimer(SmoothMoveHandle);
	GetWorld()->GetTimerManager().SetTimer(
		SmoothMoveHandle, this, &ALCWeightPlate::UpdateSmoothMove,
		MoveStepInterval, true
	);

	if (TotalWeight >= RequiredWeight)
	{
		if (bActivated==false)
		{
			ILCGimmickInterface::Execute_ActivateGimmick(this);
			bActivated = true;
		}
	}
	else
	{
		if (bActivated==true)
		{
			ILCGimmickInterface::Execute_DeactivateGimmick(this);
		}
	}
}

void ALCWeightPlate::UpdateSmoothMove()
{
	MoveElapsed += MoveStepInterval;
	float Alpha = FMath::Clamp(MoveElapsed / MoveDuration, 0.f, 1.f);
	FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
	SetActorLocation(NewLocation);

	if (Alpha >= 1.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(SmoothMoveHandle);
	}
}

float ALCWeightPlate::GetActorWeight(AActor* Actor) const
{
	TArray<UPrimitiveComponent*> Components;
	Actor->GetComponents<UPrimitiveComponent>(Components);

	float MassSum = 0.f;
	for (UPrimitiveComponent* Comp : Components)
	{
		if (Comp && Comp->IsSimulatingPhysics())
		{
			MassSum += Comp->GetMass();
		}
	}

	if (MassSum > 0.f)
	{
		return MassSum;
	}
	
	return 0.0f; // 기본값
}

void ALCWeightPlate::DeactivateGimmick_Implementation()
{
	LOG_Frame_WARNING(TEXT("[WeightPlate] Deactivated due to insufficient weight."));
	bActivated = false;

	if (bRevertible==true)
	{
		for (AActor* Target : LinkedTargets)
		{
			if (IsValid(Target)==false)
			{
				continue;
			}

			if (Target->GetClass()->ImplementsInterface(UGimmickEffectInterface::StaticClass()))
			{
				IGimmickEffectInterface::Execute_StopEffect(Target);
			}
		}
	}
}
