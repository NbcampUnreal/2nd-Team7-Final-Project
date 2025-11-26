#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LCGimmickSpawnInterface.generated.h"

UINTERFACE(BlueprintType)
class ULCGimmickSpawnInterface : public UInterface
{
	GENERATED_BODY()
};

class ILCGimmickSpawnInterface
{
	GENERATED_BODY()

public:
	/** 트리거의 타겟 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gimmick|SpawnLink")
	void SetLinkedTarget(AActor* Target);

	/** 타겟의 트리거 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gimmick|SpawnLink")
	void SetLinkedTrigger(AActor* Trigger);
};
