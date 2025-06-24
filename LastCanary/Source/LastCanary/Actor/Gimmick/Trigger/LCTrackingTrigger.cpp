#include "Actor/Gimmick/Trigger/LCTrackingTrigger.h"
#include "Actor/Gimmick/Manager/LCTrackingManager.h"
#include "Components/BoxComponent.h"
#include "Interface/InteractableInterface.h"
#include "LastCanary.h"


ALCTrackingTrigger::ALCTrackingTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALCTrackingTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (DetectionArea)
	{
		DetectionArea->OnComponentBeginOverlap.AddDynamic(this, &ALCTrackingTrigger::OnTriggerEnter);
		DetectionArea->OnComponentEndOverlap.AddDynamic(this, &ALCTrackingTrigger::OnTriggerExit);

		//LOG_Art(Log, TEXT(" 감지 트리거 활성화됨 (%s)"), *GetName());

	}
}

void ALCTrackingTrigger::OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !IsValid(OtherActor)) return;
	if (!IsValid(TrackingManager)) return;

	if (IsValidActivator(OtherActor))
	{
		//LOG_Art(Log, TEXT(" 감지 시작: %s"), *OtherActor->GetName());
		TrackingManager->RegisterPlayer(OtherActor);
	}
}

void ALCTrackingTrigger::OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority() || !IsValid(OtherActor)) return;
	if (!IsValid(TrackingManager)) return;

	if (IsValidActivator(OtherActor))
	{
		//LOG_Art(Log, TEXT(" 감지 종료: %s"), *OtherActor->GetName());
		TrackingManager->UnregisterPlayer(OtherActor);
	}
}