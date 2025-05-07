#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LCLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(Server, Reliable)
	void Server_RequestJoinRoom(const FString& RoomID, const FString& Password);
	void Server_RequestJoinRoom_Implementation(const FString& RoomID, const FString& Password);
	
};
