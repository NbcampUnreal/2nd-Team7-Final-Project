#include "Framework/GameState/LCGameState.h"
#include "Net/UnrealNetwork.h"

void ALCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCGameState, SelectedMapIndex);
}

UDataTable* ALCGameState::GetMapData()
{
	return this->MapData;
}

int32 ALCGameState::GetSelectedMapIndex()
{
	return SelectedMapIndex;
}

void ALCGameState::SetMapIndex(int32 Index)
{
	SelectedMapIndex = Index;
}


void ALCGameState::OnRep_SelectMapChanged()
{
	UE_LOG(LogTemp, Warning, TEXT("SelectedMapChanged"));
}
