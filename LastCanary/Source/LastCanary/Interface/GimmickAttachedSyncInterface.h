#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GimmickAttachedSyncInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UGimmickAttachedSyncInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LASTCANARY_API IGimmickAttachedSyncInterface
{
	GENERATED_BODY()

public:

	/** 부모 회전 시작 시 호출 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GimmickSync")
	void StartClientSyncRotation(const FQuat& From, const FQuat& To, float Duration);

	/** 부모 이동 시작 시 호출 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GimmickSync")
	void StartClientSyncMovement(const FVector& From, const FVector& To, float Duration);
};
