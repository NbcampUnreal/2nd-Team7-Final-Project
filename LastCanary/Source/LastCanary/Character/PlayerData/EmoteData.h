#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimMontage.h"
#include "EmoteData.generated.h"

USTRUCT(BlueprintType)
struct FEmoteData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* Montage;
};

UCLASS(BlueprintType)
class LASTCANARY_API UEmoteDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FEmoteData> Emotes;
};
