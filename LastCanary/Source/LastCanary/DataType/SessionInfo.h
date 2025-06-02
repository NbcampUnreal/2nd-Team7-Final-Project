#pragma once

#include "CoreMinimal.h"
#include "SessionInfo.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ServerName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPublic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxPlayerNum;
};
