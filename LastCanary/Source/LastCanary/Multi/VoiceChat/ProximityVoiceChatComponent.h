// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProximityVoiceChatComponent.generated.h"


class UVOIPTalker;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UProximityVoiceChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UProximityVoiceChatComponent();
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void ToggleSpeaking(bool bEnable);
	
protected:
	UPROPERTY()
	UVOIPTalker* VOIPTalker;
	
	void InitializeVoiceChat();
	
	void ConfigureVoiceSettings();
	
	UPROPERTY(EditAnywhere, Category = "Voice Chat")
	float MicThreshold = -1.0f;

	UPROPERTY(EditAnywhere, Category = "Voice Chat")
	USoundAttenuation* AttenuationSettings;
	
	UPROPERTY(EditAnywhere, Category = "Voice Chat")
	USoundEffectSourcePresetChain* SourceEffectChain;
};
