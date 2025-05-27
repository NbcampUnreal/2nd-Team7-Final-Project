#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Net/UnrealNetwork.h"

void ALCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCGameState, SelectedMapIndex);
	DOREPLIFETIME(ALCGameState, PlayerNum);
}

void ALCGameState::OnRep_ReplicatedHasBegunPlay()
{
	Super::OnRep_ReplicatedHasBegunPlay();

}

void ALCGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALCRoomPlayerController* LPC = Cast<ALCRoomPlayerController>(It->Get()))
		{
			LPC->Client_UpdateLobbyUI_Implementation();
		}
	}
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

void ALCGameState::SetPlayerNum()
{

}


void ALCGameState::OnRep_SelectMapChanged()
{
	UE_LOG(LogTemp, Warning, TEXT("SelectedMapChanged"));
}

void ALCGameState::OnRep_PlayerNumChanged() const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALCRoomPlayerController* LPC = Cast<ALCRoomPlayerController>(It->Get()))
		{
			LPC->Client_UpdateLobbyUI_Implementation();
		}
	}
}
