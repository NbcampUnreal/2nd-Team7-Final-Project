#include "Actor/Gimmick/Trigger/LCOrderPlate.h"
#include "Components/BoxComponent.h"
#include "Actor/Gimmick/Trigger/LCOrderSwitch.h"
#include "Interface/LCGimmickInterface.h"
#include "GameFramework/Character.h"

#include "LastCanary.h"

ALCOrderPlate::ALCOrderPlate()
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
	TriggerVolume->SetGenerateOverlapEvents(true);

	PlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
	PlateMesh->SetupAttachment(RootComponent);
}

void ALCOrderPlate::BeginPlay()
{
	Super::BeginPlay();

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ALCOrderPlate::OnBeginOverlap);

	if (DefaultMaterial)
	{
		PlateMesh->SetMaterial(0, DefaultMaterial);
	}
}

void ALCOrderPlate::ApplyMaterial(bool bIsSelected)
{
	if (PlateMesh == nullptr)
	{
		return;
	}

	UMaterialInterface* MaterialToApply = bIsSelected ? SelectedMaterial : DefaultMaterial;

	if (MaterialToApply)
	{
		PlateMesh->SetMaterial(0, MaterialToApply);
	}
}

void ALCOrderPlate::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (IsValid(TargetSwitch) == false)
	{
		return;
	}

	if (OtherActor && OtherActor->IsA<ACharacter>())
	{
		ILCGimmickInterface::Execute_ActivateGimmick(TargetSwitch);
		LOG_Frame_WARNING(TEXT("[OrderPlate] Activated switch via plate."));
	}
}
