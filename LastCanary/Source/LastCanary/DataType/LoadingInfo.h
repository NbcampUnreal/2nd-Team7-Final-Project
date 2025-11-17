#pragma once

#include "CoreMinimal.h"
#include "LoadingInfo.generated.h"


USTRUCT(BlueprintType)
struct FLoadingInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ToolTip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> LoadingThumbnail = nullptr;
};