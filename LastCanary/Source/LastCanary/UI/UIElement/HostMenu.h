// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "HostMenu.generated.h"

/**
 * 
 */
class UButton;
UCLASS()
class LASTCANARY_API UHostMenu : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	UFUNCTION()
	void OnStartButtonClicked();
};
