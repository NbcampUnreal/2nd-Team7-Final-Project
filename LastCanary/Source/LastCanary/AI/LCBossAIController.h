#pragma once

#include "CoreMinimal.h"
#include "AI/LCBaseBossAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "LCBossAIController.generated.h"

UCLASS()
class LASTCANARY_API ALCBossAIController : public ALCBaseBossAIController
{
	GENERATED_BODY()

public:
    ALCBossAIController();

protected:
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    virtual void ConfigureSenses() override;
	
};
