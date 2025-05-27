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
	TSoftObjectPtr<UWorld> MapPath;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> MapThumbnail = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Difficulty;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};