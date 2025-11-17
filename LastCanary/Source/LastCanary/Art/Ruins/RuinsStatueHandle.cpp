#include "RuinsStatueHandle.h"
#include "RuinsRotatingStatue.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"

ARuinsStatueHandle::ARuinsStatueHandle()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	CooldownTime = 1.0f;
	LastActivatedTime = -1000.f;
}

void ARuinsStatueHandle::BeginPlay()
{
	Super::BeginPlay();
}

void ARuinsStatueHandle::ActivateGimmick_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastActivatedTime < CooldownTime)
	{
		const float Remain = CooldownTime - (CurrentTime - LastActivatedTime);
		LOG_Art_WARNING(TEXT("ARuinsStatueHandle - 쿨타임 미충족: %.2f초 남음"), Remain);
		return;
	}

	LastActivatedTime = CurrentTime;

	// 핸들 자신 회전 + 사운드 재생
	StartRotation(RotationStep);
	Multicast_PlaySound();

	LOG_Art(Log, TEXT("ARuinsStatueHandle - 회전 실행됨"));

	// 동상에게 상호작용 전달 (인터페이스 기반)
	if (IsValid(TargetStatue))
	{
		if (TargetStatue->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			IInteractableInterface::Execute_Interact(TargetStatue, nullptr);
			LOG_Art(Log, TEXT("ARuinsStatueHandle - TargetStatue에 Interact 호출: %s"), *TargetStatue->GetName());
		}
		else
		{
			LOG_Art_WARNING(TEXT("ARuinsStatueHandle - TargetStatue가 IInteractableInterface 구현 안됨: %s"), *TargetStatue->GetName());
		}
	}
	else
	{
		LOG_Art_WARNING(TEXT("ARuinsStatueHandle - TargetStatue가 유효하지 않음"));
	}
}

FString ARuinsStatueHandle::GetInteractMessage_Implementation() const
{
	return TEXT("Press F");
}