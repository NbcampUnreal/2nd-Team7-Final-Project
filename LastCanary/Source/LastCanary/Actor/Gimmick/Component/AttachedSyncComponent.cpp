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

	OnComponentBeginOverlap.AddUniqueDynamic(this, &UAttachedSyncComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddUniqueDynamic(this, &UAttachedSyncComponent::OnOverlapEnd);
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
		if (!IsValid(Actor)) continue;

		FTransform ParentTransform = GetAttachParent()->GetComponentTransform();
		FTransform ActorTransform = Actor->GetActorTransform();
		FTransform RelativeTransform = ActorTransform.GetRelativeTransform(ParentTransform);

		CachedRelativeTransforms.Add(Actor, RelativeTransform);

		if (Actor->Implements<UGimmickAttachedSyncInterface>())
		{
			IGimmickAttachedSyncInterface::Execute_StartClientSyncMovement(Actor, From, To, Duration);
		}
	}
}

void UAttachedSyncComponent::BroadcastStartRotation(const FQuat& From, const FQuat& To, float Duration)
{
	LOG_Art(Log, TEXT("[AttachedSync] ▶ 회전 동기화 시작: 대상 %d개"), AttachedActors.Num());

	for (AActor* Actor : AttachedActors)
	{
		if (!IsValid(Actor)) continue;

		// 부모 VisualMesh 기준 상대 Transform 적용
		if (USceneComponent* ParentComp = GetAttachParent())
		{
			const FTransform ParentTransform = ParentComp->GetComponentTransform();
			if (const FTransform* RelativeTransform = CachedRelativeTransforms.Find(Actor))
			{
				const FTransform NewWorldTransform = (*RelativeTransform) * ParentTransform;
				Actor->SetActorTransform(NewWorldTransform);

				LOG_Art(Log, TEXT("[AttachedSync] ▶ 상대 Transform 적용: %s"), *Actor->GetName());
			}
		}

		if (Actor->Implements<UGimmickAttachedSyncInterface>())
		{
			LOG_Art(Log, TEXT("[AttachedSync] ▶ 인터페이스 호출 → StartClientSyncRotation"));
			IGimmickAttachedSyncInterface::Execute_StartClientSyncRotation(Actor, From, To, Duration);
		}
		else
		{
			LOG_Art_WARNING(TEXT("[AttachedSync] ❌ 인터페이스 미구현: %s"), *Actor->GetName());
		}
	}
}

const TArray<AActor*>& UAttachedSyncComponent::GetAttachedActors() const
{
	return AttachedActors;
}