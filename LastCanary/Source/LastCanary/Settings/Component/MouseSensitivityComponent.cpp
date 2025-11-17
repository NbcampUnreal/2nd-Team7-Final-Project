#include "Settings/Component/MouseSensitivityComponent.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"

void UMouseSensitivityComponent::BeginPlay()
{
	Super::BeginPlay();
	LoadMouseSensivity();
	LoadZoomSensivity();
	LoadDroneSensivity();
}


float UMouseSensitivityComponent::GetMouseSensivity()
{
	return MouseSensivity;
}

float UMouseSensitivityComponent::GetZoomSensivity()
{
	return ZoomSensivity;
}

float UMouseSensitivityComponent::GetDroneSensivity()
{
	return DroneSensivity;
}

void UMouseSensitivityComponent::SetMouseSensivity(float NewSensitivity)
{
	MouseSensivity = NewSensitivity;
}

void UMouseSensitivityComponent::SetZoomSensivity(float NewSensitivity)
{
	ZoomSensivity = NewSensitivity;
}

void UMouseSensitivityComponent::SetDroneSensivity(float NewSensitivity)
{
	DroneSensivity = NewSensitivity;
}

void UMouseSensitivityComponent::LoadMouseSensivity()
{
	MouseSensivity = ULCLocalPlayerSaveGame::LoadMouseSensitivity(GetWorld());
}

void UMouseSensitivityComponent::LoadZoomSensivity()
{
	ZoomSensivity = ULCLocalPlayerSaveGame::LoadZoomSensitivity(GetWorld());
}

void UMouseSensitivityComponent::LoadDroneSensivity()
{
	DroneSensivity = ULCLocalPlayerSaveGame::LoadDroneSensitivity(GetWorld());
}
