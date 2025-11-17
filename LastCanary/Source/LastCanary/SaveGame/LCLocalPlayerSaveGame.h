#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "UI/UIObject/VoiceOptionWidget.h"
#include "LCLocalPlayerSaveGame.generated.h"

struct FEnhancedActionKeyMapping;

namespace DefaultSettings
{
	constexpr float DEFAULT_SENSITIVITY = 1.0f;
	constexpr float DEFAULT_VOLUME = 1.0f;
	constexpr float DEFAULT_BRIGHTNESS = 1.0f;
	constexpr bool DEFAULT_FULLSCREEN = true;
	constexpr EVoiceInputMode DEFAULT_VOICEMODE = EVoiceInputMode::Always;
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
	float DroneSensitivity = DefaultSettings::DEFAULT_SENSITIVITY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoomSensitivity = DefaultSettings::DEFAULT_SENSITIVITY;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVoiceInputMode VoiceInputMode = DefaultSettings::DEFAULT_VOICEMODE;

	UPROPERTY()
	TArray<FSaveKeyMapping> SavedMappings;	

	void ResetToDefault()
	{
		MouseSensitivity = DefaultSettings::DEFAULT_SENSITIVITY;
		DroneSensitivity = DefaultSettings::DEFAULT_SENSITIVITY;
		ZoomSensitivity = DefaultSettings::DEFAULT_SENSITIVITY;
		MasterVolume = DefaultSettings::DEFAULT_VOLUME;
		BGMVolume = DefaultSettings::DEFAULT_VOLUME;
		EffectVolume = DefaultSettings::DEFAULT_VOLUME;
		Brightness = DefaultSettings::DEFAULT_BRIGHTNESS;
		bIsFullScreenMode = DefaultSettings::DEFAULT_FULLSCREEN;
		VoiceChatVolume = DefaultSettings::DEFAULT_VOLUME;
		MicrophoneVolume = DefaultSettings::DEFAULT_VOLUME;
		VoiceInputMode = DefaultSettings::DEFAULT_VOICEMODE;
	}
};

USTRUCT(BlueprintType)
struct FCharacterCustomizationData
{
	GENERATED_BODY()
	// SkeletalMesh는 직접 Replicate 안되므로 ID 방식이 필요함

	//메시 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DefaultBodyID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GloveID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 JacketID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PantsID = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BeltsID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HelmetID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ArmorID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BootsID = 0;


	//머티리얼 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DefaultBodyMaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GloveMaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 JacketMaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PantsMaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BeltsMaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HelmetMaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ArmorMaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BootsMaterialID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FlagMaterialID = 0;

	FCharacterCustomizationData() {}
	FCharacterCustomizationData(int32 DefaultBody, int32 Glove, int32 Jacket, int32 Pants, int32 Belts, int32 Helmet, int32 Armor, int32 Boots)
		: DefaultBodyID(DefaultBody), GloveID(Glove), JacketID(Jacket), PantsID(Pants), BeltsID(Belts), HelmetID(Helmet), ArmorID(Armor), BootsID(Boots) {
	}
	FCharacterCustomizationData(
		int32 DefaultBody, int32 Glove, int32 Jacket, int32 Pants, int32 Belts, int32 Helmet, int32 Armor, int32 Boots,
		int32 DefaultBodyMat, int32 GloveMat, int32 JacketMat, int32 PantsMat, int32 BeltsMat, int32 HelmetMat, int32 ArmorMat, int32 BootsMat, int32 FlagMat
	)
		: DefaultBodyID(DefaultBody), GloveID(Glove), JacketID(Jacket), PantsID(Pants), BeltsID(Belts), HelmetID(Helmet), ArmorID(Armor), BootsID(Boots),
		DefaultBodyMaterialID(DefaultBodyMat), GloveMaterialID(GloveMat), JacketMaterialID(JacketMat), PantsMaterialID(PantsMat),
		BeltsMaterialID(BeltsMat), HelmetMaterialID(HelmetMat), ArmorMaterialID(ArmorMat), BootsMaterialID(BootsMat), FlagMaterialID(FlagMat)
	{
	}
};


UCLASS()
class LASTCANARY_API ULCLocalPlayerSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUserSettings Settings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCharacterCustomizationData CustomizationData;

public:

	static bool SaveMouseSensitivity(UWorld* World, float NewSensitivity);
	static float LoadMouseSensitivity(UWorld* World);

	static bool SaveDroneSensitivity(UWorld* World, float NewSensitivity);
	static float LoadDroneSensitivity(UWorld* World);

	static bool SaveZoomSensitivity(UWorld* World, float NewSensitivity);
	static float LoadZoomSensitivity(UWorld* World);

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

	static bool SaveVoiceInputMode(UWorld* World, EVoiceInputMode NewSetting);
	static EVoiceInputMode LoadVoiceInputMode(UWorld* World);

	static bool SaveKeyBindings(UWorld* World, const TArray<FSaveKeyMapping>& Mappings);
	static TArray<FSaveKeyMapping> LoadKeyBindings(UWorld* World);

	static bool SaveCustomizationData(UWorld* World, const FCharacterCustomizationData NewSetting);
	static FCharacterCustomizationData LoadCustomizationData(UWorld* World);

	static ULCLocalPlayerSaveGame* GetSaveInstance(UWorld* World);
};
