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
		if (!IsValid(Actor)) continue;

		const FQuat ActorQuat = Actor->GetActorQuat(); // 현재 액터 회전
		const FQuat DeltaQuat = To * From.Inverse();   // 부모 기준 회전 변화량
		const FQuat TargetQuat = DeltaQuat * ActorQuat; // 액터 기준 목표 회전

		// 디버깅 로그
		LOG_Art(Log, TEXT("[AttachedSync] ▶ 대상: %s"), *Actor->GetName());
		LOG_Art(Log, TEXT(" └ 현재 회전:        %s"), *ActorQuat.Rotator().ToCompactString());
		LOG_Art(Log, TEXT(" └ 부모 기준 From:    %s"), *From.Rotator().ToCompactString());
		LOG_Art(Log, TEXT(" └ 부모 기준 To:      %s"), *To.Rotator().ToCompactString());
		LOG_Art(Log, TEXT(" └ 계산된 TargetQuat: %s"), *TargetQuat.Rotator().ToCompactString());

		// ✅ SetActorRotation 제거됨
		// ❌ 절대 FromRot으로 강제 초기화하지 말 것!

		if (Actor->Implements<UGimmickAttachedSyncInterface>())
		{
			LOG_Art(Log, TEXT(" ▶ 인터페이스 실행 → StartClientSyncRotation"));
			IGimmickAttachedSyncInterface::Execute_StartClientSyncRotation(Actor, ActorQuat, TargetQuat, Duration);
		}
		else
		{
			LOG_Art_WARNING(TEXT(" ▶ ❌ 인터페이스 미구현: %s"), *Actor->GetName());
		}
	}
}

const TArray<AActor*>& UAttachedSyncComponent::GetAttachedActors() const
{
	return AttachedActors;
}