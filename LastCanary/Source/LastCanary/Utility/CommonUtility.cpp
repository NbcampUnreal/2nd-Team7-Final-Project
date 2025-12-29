#include "Utility/CommonUtility.h"
#include "TimerManager.h"


void UCommonUtility::RetryUntilValid(UWorld* World, TFunction<bool()> IsValidFunc, TFunction<void()> ExecuteFunc, float RetryTime)
{
	if (!World)
		return;

	if (IsValidFunc())
	{
		ExecuteFunc();
		return;
	}

	// 조건이 false면 타이머로 재시도
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		[World, IsValidFunc, ExecuteFunc, RetryTime]()
		{
			RetryUntilValid(World, IsValidFunc, ExecuteFunc, RetryTime);
		},
		RetryTime,
		false
	);
}