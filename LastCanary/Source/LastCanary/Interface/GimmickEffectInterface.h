#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GimmickEffectInterface.generated.h"

UINTERFACE(MinimalAPI)
class UGimmickEffectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 출력 담당
 * 기믹을 받아서 반응하는 쪽에 붙는 인터페이스
 */
class LASTCANARY_API IGimmickEffectInterface
{
	GENERATED_BODY()

public:
    /** 기믹 효과가 발동될 때 호출되는 함수 */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gimmick")
    void TriggerEffect();
};
