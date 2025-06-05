#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LCGameMode.generated.h"

struct FSessionPlayerInfo;

UCLASS()
class LASTCANARY_API ALCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void SpawnPlayerCharacter(APlayerController* Controller);

	void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable)
	void KickPlayer(const FSessionPlayerInfo& SessionInfo, const FText& KickReason);


	void SetPlayerInfo(const FSessionPlayerInfo& RequestInfo);

	void UpdatePlayers();

	bool IsAllPlayersReady() const;

private:
	TArray<APlayerController*> AllPlayerControllers;
	TArray<FSessionPlayerInfo> SessionPlayerInfos;

	void CachingNewPlayer(APlayerController* NewPlayer);
	void RemoveCachedPlayer(APlayerController* PC);

	int MaxPlayerNum = 4;
	int CurrentPlayerNum = 0;

public:
	// Server Travel Logic
	UFUNCTION(BlueprintCallable, Category = "Session")
	void TravelMapBySoftPath(FString SoftPath);
	UFUNCTION(BlueprintCallable, Category = "Session")
	void TravelMapByPath(FString Path);

	void ShowLoading();

protected:
	void SendMessageToAllPC(const FString& Message);

};