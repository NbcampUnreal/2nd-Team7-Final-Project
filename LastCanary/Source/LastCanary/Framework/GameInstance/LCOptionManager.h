#pragma once

#include "CoreMinimal.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "LCOptionManager.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EScreenMode : uint8
{
	FullScreen UMETA(DisplayName = "FullScreen"),
	Window UMETA(DisplayName = "Window")
};

UCLASS()
class LASTCANARY_API ULCOptionManager : public ULCGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintReadWrite, Category = "Option")
	float MasterVolume = 0.5f;
	UPROPERTY(BlueprintReadWrite, Category = "Option")
	float BGMVolume = 0.5f;
	UPROPERTY(BlueprintReadWrite, Category = "Option")
	float EffectVolume = 0.5f;

	UPROPERTY(BlueprintReadWrite, Category = "Option")
	float MouseSensitivity = 0.5f;
	UPROPERTY(BlueprintReadWrite, Category = "Option")
	float Brightness = 0.5f;

	UPROPERTY(BlueprintReadWrite, Category = "Option")
	EScreenMode ScreenMode;
	UPROPERTY(BlueprintReadWrite, Category = "Option")
	int32 ResolutionIndex = 2;
	UPROPERTY(BlueprintReadWrite, Category = "Option")
	FIntPoint ScreenResolution = FIntPoint(1920, 1080);

	UFUNCTION()
	void ChangeScreen(EScreenMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Option")
	void ApplyOptions();

	//TODO : 저장 / 불러오기 함수
};
