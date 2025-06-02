#include "RuinsStatueHandle.h"
#include "RuinsRotatingStatue.h"
#include "Net/UnrealNetwork.h"
#include <Kismet/GameplayStatics.h>
#include "TimerManager.h"
#include "LastCanary.h"

ARuinsStatueHandle::ARuinsStatueHandle()
	: CooldownTime(1.0f) 
	, LastActivatedTime(-1000.f) 
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

}

void ARuinsStatueHandle::ActivateGimmick_Implementation()
{
	Super::ActivateGimmick_Implementation();

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastActivatedTime < CooldownTime)
	{
		LOG_Art_WARNING(TEXT("핸들 쿨타임 진행 중 - %.2f초 남음"), CooldownTime - (CurrentTime - LastActivatedTime));
		return;
	}

	if (!IsValid(TargetStatue))
	{
		LOG_Art_ERROR(TEXT("TargetStatue가 유효하지 않습니다."));
		return;
	}

	// 회전 동상으로 캐스팅 시도
	ARuinsRotatingStatue* Statue = Cast<ARuinsRotatingStatue>(TargetStatue);
	if (!IsValid(Statue))
	{
		LOG_Art_WARNING(TEXT("TargetStatue는 ARuinsRotatingStatue 타입이 아닙니다."));
		return;
	}

	// 동상 회전 요청
	Statue->ActivateGimmick();

	LastActivatedTime = CurrentTime;

	LOG_Art(Log, TEXT("핸들이 동상 회전 요청을 보냈습니다."));
}

// 상호작용 메시지 반환
FString ARuinsStatueHandle::GetInteractMessage_Implementation() const
{
	return TEXT("핸들을 돌려 동상을 회전시킵니다");
}
