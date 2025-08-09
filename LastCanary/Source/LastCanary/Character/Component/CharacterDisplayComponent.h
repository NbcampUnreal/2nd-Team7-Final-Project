// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterDisplayComponent.generated.h"

class UPostProcessComponent;

UCLASS()
class LASTCANARY_API UCharacterDisplayComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
	UCharacterDisplayComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 밝기 설정
	void ApplyBrightness(float Value);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Brightness")
	float MinBrightness = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Brightness")
	float MaxBrightness = 10.0f;

	UPROPERTY()
	UPostProcessComponent* PostProcessComponent;

	void InitializePostProcess();

	void LoadBrightnessSetting();
};
