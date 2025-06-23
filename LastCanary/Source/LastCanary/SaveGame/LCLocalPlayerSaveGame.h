#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "LCLocalPlayerSaveGame.generated.h"


struct FEnhancedActionKeyMapping;

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
struct FSaveKeyMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MappingName;  // 매핑 이름 (액션의 FName)

	UPROPERTY()
	FKey Key;

	FSaveKeyMapping() {}
	FSaveKeyMapping(FName InMappingName, FKey InKey) : MappingName(InMappingName), Key(InKey) {}
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VoiceChatVolume = DefaultSettings::DEFAULT_VOLUME;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MicrophoneVolume = DefaultSettings::DEFAULT_VOLUME;

	UPROPERTY()
	TArray<FSaveKeyMapping> SavedMappings;

	void ResetToDefault()
	{
		MouseSensitivity = 1.0f;
		MasterVolume = 1.0f;
		BGMVolume = 1.0f;
		EffectVolume = 1.0f;
		Brightness = 1.0f;
		bIsFullScreenMode = true;
		VoiceChatVolume = 1.0f;
		MicrophoneVolume = 1.0f;
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

	static bool SaveMicrophoneVolume(UWorld* World, float NewVolume);
	static float LoadMicrophoneVolume(UWorld* World);

	static bool SaveVoiceChatVolume(UWorld* World, float NewVolume);
	static float LoadVoiceChatVolume(UWorld* World);

	static bool SaveMicrophoneMode(UWorld* World, bool NewSetting);
	static bool LoadMicrophoneMode(UWorld* World);

	static bool SaveKeyBindings(UWorld* World, const TArray<FSaveKeyMapping>& Mappings);
	static TArray<FSaveKeyMapping> LoadKeyBindings(UWorld* World);

	static ULCLocalPlayerSaveGame* GetSaveInstance(UWorld* World);
};
