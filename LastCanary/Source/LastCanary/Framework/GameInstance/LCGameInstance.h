#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LCGameInstance.generated.h"

/**
 * 
 */
class ULCUIManagerSettings;
UCLASS()
class LASTCANARY_API ULCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, Category = "UI")
	ULCUIManagerSettings* UIManagerSettings;

public:
	ULCUIManagerSettings* GetUIManagerSettings() const;

};
