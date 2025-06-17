#include "Actor/Gimmick/Component/AttachedSyncComponent.h"
#include "Interface/GimmickAttachedSyncInterface.h"
#include "LastCanary.h"

UAttachedSyncComponent::UAttachedSyncComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttachedSyncComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UAttachedSyncComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &UAttachedSyncComponent::OnOverlapEnd);
}

void UAttachedSyncComponent::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner()) return;
	if (!OtherActor->ActorHasTag(TargetActorTag)) return;

	if (!AttachedActors.Contains(OtherActor))
	{
		AttachedActors.Add(OtherActor);
		LOG_Art(Log, TEXT("[AttachedSync] ▶ 감지 시작: %s"), *OtherActor->GetName());
	}
}

void UAttachedSyncComponent::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AttachedActors.Contains(OtherActor))
	{
		AttachedActors.Remove(OtherActor);
		LOG_Art(Log, TEXT("[AttachedSync] ❌ 감지 해제: %s"), *OtherActor->GetName());
	}
}

void UAttachedSyncComponent::BroadcastStartMovement(const FVector& From, const FVector& To, float Duration)
{
	for (AActor* Actor : AttachedActors)
	{
		if (IsValid(Actor) && Actor->Implements<UGimmickAttachedSyncInterface>())
		{
			IGimmickAttachedSyncInterface::Execute_StartClientSyncMovement(Actor, From, To, Duration);
		}
	}
}

void UAttachedSyncComponent::BroadcastStartRotation(const FQuat& From, const FQuat& To, float Duration)
{
	LOG_Art(Log, TEXT("[AttachedSync] ▶ 회전 동기화 브로드캐스트 시작: 대상 %d개"), AttachedActors.Num());

	for (AActor* Actor : AttachedActors)
	{
		if (IsValid(Actor))
		{
			if (Actor->Implements<UGimmickAttachedSyncInterface>())
			{
				LOG_Art(Log, TEXT("[AttachedSync] ▶ 회전 동기화 실행 대상: %s"), *Actor->GetName());
				IGimmickAttachedSyncInterface::Execute_StartClientSyncRotation(Actor, From, To, Duration);
			}
			else
			{
				LOG_Art_WARNING(TEXT("[AttachedSync] ❌ 인터페이스 미구현 대상: %s"), *Actor->GetName());
			}
		}
	}
}

const TArray<AActor*>& UAttachedSyncComponent::GetAttachedActors() const
{
	return AttachedActors;
}