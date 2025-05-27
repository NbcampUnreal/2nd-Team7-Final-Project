#pragma once

#include "CoreMinimal.h"
#include "Framework/GameMode/BaseGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "LCRoomGameMode.generated.h"

UCLASS()
class LASTCANARY_API ALCRoomGameMode : public ABaseGameMode
{
	GENERATED_BODY()
	
public:
	ALCRoomGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;


};
