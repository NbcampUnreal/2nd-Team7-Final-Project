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
}

void ALCWeightPlate::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !IsValid(OtherActor))
	{
		return;
	}

	OverlappingActors.AddUnique(OtherActor);
	CheckWeight();
}

void ALCWeightPlate::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority() || !IsValid(OtherActor))
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

	if (TotalWeight >= RequiredWeight)
	{
		if (!bActivated)
		{
			ILCGimmickInterface::Execute_ActivateGimmick(this);
			bActivated = true;
		}
	}
	else
	{
		if (bActivated)
		{
			ILCGimmickInterface::Execute_DeactivateGimmick(this);
		}
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
	
	// 태그 기반 방식 (기본 사용)
	/*if (Actor->Tags.Contains(FName("Heavy")))
	{
		return 100.0f;
	}
	else if (Actor->Tags.Contains(FName("Medium")))
	{
		return 50.0f;
	}
	else if (Actor->Tags.Contains(FName("Light")))
	{
		return 10.0f;
	}*/

	return 0.0f; // 기본값
}

void ALCWeightPlate::DeactivateGimmick_Implementation()
{
	LOG_Frame_WARNING(TEXT("[WeightPlate] Deactivated due to insufficient weight."));
	bActivated = false;

	for (AActor* Target : LinkedTargets)
	{
		if (!IsValid(Target))
		{
			continue;
		}

		if (Target->GetClass()->ImplementsInterface(UGimmickEffectInterface::StaticClass()))
		{
			IGimmickEffectInterface::Execute_StopEffect(Target);
		}
	}
}
