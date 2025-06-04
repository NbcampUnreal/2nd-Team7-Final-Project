#pragma once

#include "CoreMinimal.h"
#include "SessionPlayerInfo.generated.h"

USTRUCT(BlueprintType)
struct FSessionPlayerInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlayerReady;
};
