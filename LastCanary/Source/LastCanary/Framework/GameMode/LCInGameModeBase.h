#pragma once

#include "CoreMinimal.h"
#include "Framework/GameMode/LCGameMode.h"
#include "LCInGameModeBase.generated.h"

class AChecklistManager;

UCLASS()
class LASTCANARY_API ALCInGameModeBase : public ALCGameMode
{
	GENERATED_BODY()
	
public:
	ALCInGameModeBase();

	virtual void StartPlay() override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Checklist")
	TSoftClassPtr<AChecklistManager> ChecklistManagerClass;

	UPROPERTY()
	AChecklistManager* ChecklistManager;
};
