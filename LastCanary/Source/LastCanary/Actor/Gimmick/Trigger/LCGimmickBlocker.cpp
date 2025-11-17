#include "LCGimmickBlocker.h"
#include "Components/BoxComponent.h"
#include "LastCanary.h"

ALCGimmickBlocker::ALCGimmickBlocker()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	BlockVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockVolume"));
	RootComponent = BlockVolume;

	BlockVolume->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	BlockVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BlockVolume->SetCollisionObjectType(ECC_WorldStatic);
	BlockVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	BlockVolume->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	BlockVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	BlockVolume->OnComponentBeginOverlap.AddDynamic(this, &ALCGimmickBlocker::OnBlockVolumeBeginOverlap);
}

void ALCGimmickBlocker::BeginPlay()
{
	Super::BeginPlay();
}

void ALCGimmickBlocker::OnBlockVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor)) return;

	// ObjectType 로그 출력
	const FString ObjectType = UEnum::GetValueAsString(OtherComp->GetCollisionObjectType());
	LOG_Art(Log, TEXT("▶ Blocker Overlap ▶ OtherActor: %s | Comp: %s | ObjectType: %s"),
		*GetNameSafe(OtherActor),
		*GetNameSafe(OtherComp),
		*ObjectType);
}
