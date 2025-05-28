#include "Actor/Gimmick/Trigger/LCSimultaneousPlate.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

#include "LastCanary.h"

ALCSimultaneousPlate::ALCSimultaneousPlate()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(100.f, 100.f, 50.f));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerVolume->SetGenerateOverlapEvents(true);
}

void ALCSimultaneousPlate::BeginPlay()
{
    Super::BeginPlay();

    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ALCSimultaneousPlate::OnBeginOverlap);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ALCSimultaneousPlate::OnEndOverlap);
}

void ALCSimultaneousPlate::OnBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex,
    bool bFromSweep, 
    const FHitResult& SweepResult)
{
    if (!HasAuthority() || !IsValid(OtherActor))
    {
        return;
    }
    if (Cast<ACharacter>(OtherActor) == nullptr)
    {
        return; // 캐릭터만 인정
    }

    OverlappingActors.Add(OtherActor);

    LOG_Frame_WARNING(
        TEXT("[Plate] Overlap Begin: %s (Count: %d / %d)"),
        *OtherActor->GetName(),
        OverlappingActors.Num(),
        RequiredCount
    );

    CheckActivation();
}

void ALCSimultaneousPlate::OnEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex)
{
    if (!HasAuthority() || !IsValid(OtherActor))
    {
        return;
    }

    OverlappingActors.Remove(OtherActor);
    // 재진입 허용할 경우: bActivated = false; 체크

    LOG_Frame_WARNING(TEXT("[Plate] Overlap End: %s (Count: %d / %d)"),
        *OtherActor->GetName(), OverlappingActors.Num(), RequiredCount);
}

bool ALCSimultaneousPlate::CanActivate_Implementation()
{
    return OverlappingActors.Num() >= RequiredCount;
}

void ALCSimultaneousPlate::CheckActivation()
{
    if (bActivated)
    {
        return;
    }

    if (ILCGimmickInterface::Execute_CanActivate(this))
    {
        LOG_Frame_WARNING(TEXT("[Plate] Activation condition met. Triggering gimmick."));
        ILCGimmickInterface::Execute_ActivateGimmick(this);
    }
}
