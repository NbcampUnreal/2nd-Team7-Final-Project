#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GimmickDebuffInterface.generated.h"

UINTERFACE(MinimalAPI)
class UGimmickDebuffInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LASTCANARY_API IGimmickDebuffInterface
{
	GENERATED_BODY()

public:
	/** 이동속도 감소 디버프 적용 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GimmickDebuff")
	void ApplyMovementDebuff(float SlowRate, float Duration);

	/** 이동속도 감소 디버프 해제 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GimmickDebuff")
	void RemoveMovementDebuff();

};
