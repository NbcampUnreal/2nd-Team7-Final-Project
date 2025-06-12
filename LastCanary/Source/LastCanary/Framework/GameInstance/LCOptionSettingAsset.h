#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "LCOptionSettingAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class LASTCANARY_API ULCOptionSettingAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundMix* AudioMix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundClass* MasterSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundClass* BGMSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundClass* EffectSoundClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundClass* MyMicSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundClass* VoiceSoundClass;
};
