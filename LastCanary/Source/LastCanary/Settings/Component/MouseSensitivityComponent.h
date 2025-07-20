// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Settings/Component/PlayerSettingsComponent.h"
#include "MouseSensitivityComponent.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API UMouseSensitivityComponent : public UPlayerSettingsComponent
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

public:
	/*감도 Settings*/
	float MouseSensivity = 1.0f;
	float ZoomSensivity = 1.0f;
	float DroneSensivity = 1.0f;
	
	float GetMouseSensivity();
	float GetZoomSensivity();
	float GetDroneSensivity();

	void SetMouseSensivity(float NewSensitivity);
	void SetZoomSensivity(float NewSensitivity);
	void SetDroneSensivity(float NewSensitivity);

	void LoadMouseSensivity();
	void LoadZoomSensivity();
	void LoadDroneSensivity();
};
