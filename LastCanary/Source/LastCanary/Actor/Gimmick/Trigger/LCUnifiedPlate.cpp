//#include "Actor/Gimmick/Trigger/LCUnifiedPlate.h"
//#include "Components/BoxComponent.h"
//#include "Character/BaseCharacter.h"
//#include "Interface/GimmickEffectInterface.h"
//#include "Interface/LCGimmickInterface.h"
//
//#include "LastCanary.h"
//
//ALCUnifiedPlate::ALCUnifiedPlate()
//{
//	PrimaryActorTick.bCanEverTick = false;
//	bReplicates = true;
//
//	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
//	RootComponent = TriggerVolume;
//	TriggerVolume->SetBoxExtent(FVector(100.f));
//	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
//	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
//	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
//	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
//	TriggerVolume->SetGenerateOverlapEvents(true);
//}
//
//void ALCUnifiedPlate::BeginPlay()
//{
//	Super::BeginPlay();
//
//	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ALCUnifiedPlate::OnBeginOverlap);
//	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ALCUnifiedPlate::OnEndOverlap);
//}
//
//void ALCUnifiedPlate::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
//	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
//	bool bFromSweep, const FHitResult& SweepResult)
//{
//	if (HasAuthority() == false)
//	{
//		return;
//	}
//	if (IsValid(OtherActor) == false)
//	{
//		return;
//	}
//	if (Cast<ABaseCharacter>(OtherActor) == nullptr)
//	{
//		return;
//	}
//
//	OverlappingActors.Add(OtherActor);
//	UpdateActivationState();
//}
//
//void ALCUnifiedPlate::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
//	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if (HasAuthority() == false)
//	{
//		return;
//	}
//	if (IsValid(OtherActor) == false)
//	{
//		return;
//	}
//
//	OverlappingActors.Remove(OtherActor);
//	UpdateActivationState();
//}
//
//void ALCUnifiedPlate::UpdateActivationState()
//{
//	const int32 Count = OverlappingActors.Num();
//
//	switch (ActivationType)
//	{
//	case EGimmickActivationType::ActivateOnStep:
//		if (!bActivated && Count >= RequiredCount)
//		{
//			ILCGimmickInterface::Execute_ActivateGimmick(this);
//		}
//		break;
//	case EGimmickActivationType::ActivateWhileStepping:
//		if (Count >= RequiredCount && !bActivated)
//		{
//			ILCGimmickInterface::Execute_ActivateGimmick(this);
//		}
//		else if (Count < RequiredCount && bActivated)
//		{
//			ILCGimmickInterface::Execute_DeactivateGimmick(this);
//		}
//		break;
//	case EGimmickActivationType::ActivateAfterDelay:
//		if (Count >= RequiredCount && !GetWorld()->GetTimerManager().IsTimerActive(ActivationDelayHandle))
//		{
//			GetWorld()->GetTimerManager().SetTimer(ActivationDelayHandle, this, &ALCUnifiedPlate::TryActivateAfterDelay, ActivationDelay, false);
//		}
//		else if (Count < RequiredCount)
//		{
//			GetWorld()->GetTimerManager().ClearTimer(ActivationDelayHandle);
//		}
//		break;
//	case EGimmickActivationType::ActivateTimed:
//		if (!bActivated && Count >= RequiredCount)
//		{
//			ILCGimmickInterface::Execute_ActivateGimmick(this);
//			GetWorld()->GetTimerManager().SetTimer(ActiveDurationHandle, this, &ALCUnifiedPlate::OnTimedActivationExpired, ActiveDuration, false);
//		}
//		break;
//	}
//}
//
//void ALCUnifiedPlate::TryActivateAfterDelay()
//{
//	if (OverlappingActors.Num() >= RequiredCount && bActivated == false)
//	{
//		ILCGimmickInterface::Execute_ActivateGimmick(this);
//	}
//}
//
//void ALCUnifiedPlate::OnTimedActivationExpired()
//{
//	ILCGimmickInterface::Execute_DeactivateGimmick(this);
//}
//
//void ALCUnifiedPlate::DeactivateGimmick_Implementation()
//{
//	if (HasAuthority() == false)
//	{
//		return;
//	}
//	if (bActivated == false)
//	{
//		return;
//	}
//
//	bActivated = false;
//
//	GetWorld()->GetTimerManager().ClearTimer(ActiveDurationHandle);
//	GetWorld()->GetTimerManager().ClearTimer(ActivationDelayHandle);
//
//	for (AActor* Target : LinkedTargets)
//	{
//		if (IsValid(Target) == false)
//		{
//			continue;
//		}
//
//		if (Target->GetClass()->ImplementsInterface(UGimmickEffectInterface::StaticClass()))
//		{
//			IGimmickEffectInterface::Execute_StopEffect(Target);
//		}
//	}
//}
