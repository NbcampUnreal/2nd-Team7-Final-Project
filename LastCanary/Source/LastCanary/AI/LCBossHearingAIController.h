#pragma once

#include "CoreMinimal.h"
#include "AI/LCBaseBossAIController.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "LCBossHearingAIController.generated.h"

UCLASS()
class LASTCANARY_API ALCBossHearingAIController : public ALCBaseBossAIController
{
	GENERATED_BODY()

public:
    ALCBossHearingAIController();

protected:
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;
	
};
