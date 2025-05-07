#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LCGameInstanceSubsystem.generated.h"

/**
 * 
 */
class ULCUIManager;
UCLASS()
class LASTCANARY_API ULCGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	ULCUIManager* LCUIManager;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	ULCUIManager* GetUIManager() const;
};
