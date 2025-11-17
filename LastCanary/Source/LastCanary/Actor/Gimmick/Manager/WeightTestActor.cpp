#include "Actor/Gimmick/Manager/WeightTestActor.h"
#include "Components/StaticMeshComponent.h"

#include "LastCanary.h"

AWeightTestActor::AWeightTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

void AWeightTestActor::BeginPlay()
{
	Super::BeginPlay();
	UpdateMass();
}

void AWeightTestActor::UpdateMass()
{
	if (Mesh && Mesh->IsSimulatingPhysics())
	{
		// 질량을 수동으로 설정
		Mesh->SetMassOverrideInKg(NAME_None, CustomMass, true);
		//LOG_Frame_WARNING(TEXT("[WeightTestActor] Mass overridden to: %.1f kg"), CustomMass);
	}
}
