#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LCGimmickInterface.generated.h"

UINTERFACE(MinimalAPI)
class ULCGimmickInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 입력 담당
 * 기믹을 트리거하는 쪽에 붙는 인터페이스 (발판, 스위치 등)
 */
class LASTCANARY_API ILCGimmickInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gimmick")
	void ActivateGimmick();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gimmick")
	bool CanActivate(); 

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gimmick")
	void DeactivateGimmick();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gimmick")
	bool IsGimmickBusy();
};
