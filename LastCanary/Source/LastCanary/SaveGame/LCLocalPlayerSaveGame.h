#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LCLocalPlayerSaveGame.generated.h"



namespace DefaultSettings
{
	constexpr float DEFAULT_SENSITIVITY = 1.0f;
	constexpr float DEFAULT_VOLUME = 1.0f;
	constexpr float DEFAULT_BRIGHTNESS = 1.0f;
	constexpr bool DEFAULT_FULLSCREEN = true;
}

class UMySaveGameUtils
{
public:
	static const TCHAR PlayerSaveSlotPrefix[];  // 배열로 선언

	static FString MakeSlotName(int32 PlayerId)
	{
		return FString(PlayerSaveSlotPrefix) + FString::FromInt(PlayerId);
	}
};

USTRUCT(BlueprintType)
struct FUserSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MouseSensitivity = DefaultSettings::DEFAULT_SENSITIVITY;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MasterVolume = DefaultSettings::DEFAULT_VOLUME;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BGMVolume = DefaultSettings::DEFAULT_VOLUME;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectVolume = DefaultSettings::DEFAULT_VOLUME;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Brightness = DefaultSettings::DEFAULT_BRIGHTNESS;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFullScreenMode = DefaultSettings::DEFAULT_FULLSCREEN;

	void ResetToDefault()
	{
		MouseSensitivity = 1.0f;
		MasterVolume = 1.0f;
		BGMVolume = 1.0f;
		EffectVolume = 1.0f;
		Brightness = 1.0f;
		bIsFullScreenMode = true;
	}
};


UCLASS()
class LASTCANARY_API ULCLocalPlayerSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUserSettings Settings;


public:

	static bool SaveMouseSensitivity(UWorld* World, float NewSensitivity);
	static float LoadMouseSensitivity(UWorld* World);

	static bool SaveMasterVolume(UWorld* World, float NewVolume);
	static float LoadMasterVolume(UWorld* World);

	static bool SaveBGMVolume(UWorld* World, float NewVolume);
	static float LoadBGMVolume(UWorld* World);

	static bool SaveEffectVolume(UWorld* World, float NewVolume);
	static float LoadEffectVolume(UWorld* World);

	static bool SaveBrightness(UWorld* World, float NewBrightness);
	static float LoadBrightness(UWorld* World);

	static bool SaveFullScreenMode(UWorld* World, bool NewSetting);
	static bool LoadFullScreenMode(UWorld* World);

	static ULCLocalPlayerSaveGame* GetSaveInstance(UWorld* World);
};
