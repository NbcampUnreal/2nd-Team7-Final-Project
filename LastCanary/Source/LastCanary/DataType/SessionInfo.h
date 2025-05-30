#pragma once

#include "CoreMinimal.h"
#include "SessionInfo.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayerCount;
};
