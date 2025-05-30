#include "Framework/GameState/LCGameState.h"


void ALCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void ALCGameState::OnRep_ReplicatedHasBegunPlay()
{
	Super::OnRep_ReplicatedHasBegunPlay();

}

void ALCGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}
