// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "LC_CameraManager.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALC_CameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
protected:
	virtual void UpdateCamera(float DeltaTime) override;
public:
	virtual void UpdateViewTargetInternal(
		FTViewTarget& OutVT,
		float DeltaTime
	) override;
};
