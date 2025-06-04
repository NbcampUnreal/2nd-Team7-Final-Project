#pragma once

#include "CoreMinimal.h"
#include "DataTable/MapDataRow.h"
#include "GameFramework/GameStateBase.h"
#include "LCGameState.generated.h"

UCLASS()
class LASTCANARY_API ALCGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_ReplicatedHasBegunPlay() override;

	virtual void PostInitializeComponents() override;
};
