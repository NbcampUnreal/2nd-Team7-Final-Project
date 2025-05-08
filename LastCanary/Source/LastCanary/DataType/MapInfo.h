#pragma once

#include "CoreMinimal.h"
#include "MapInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMapInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MapName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapPath;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> MapThumbnail;
};