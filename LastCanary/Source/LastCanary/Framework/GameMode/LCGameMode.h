#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LCGameMode.generated.h"

struct FSessionPlayerInfo;

UCLASS()
class LASTCANARY_API ALCGameMode : public AGameModeBase
{
	GENERATED_BODY()

	// Open Level => 레벨 전환되면 새로운 GameMode 가 Instance 됨
	// Host가 입장 하면서 : PostLogIn -> HandleStartingNewPlayer -> StartPlay -> BeginPlay
	// Client가 입장시 : PreLogin -> PostLogin -> HandleStartingNewPlayer
	
	// SeamlessTravel 시 : PostSeamlessTravel -> HandleSeamlessTravelPlayer(호스트입장) -> HandleStartingNewPlayer(호스트입장) -> StartPlay -> Begin Play
	// HandleSeamlessTravelPlayer(Client 입장) -> HandleStartingNewPlayer(Client 입장)

public:
	void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	virtual void PostSeamlessTravel() override;

	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// GameModeBase의 StartPlay -> BeginPlay와 같은 역할을 함
	virtual void StartPlay() override;

	// Actor의 BeginPlay -> StartPlay 에서 호출 함
	virtual void BeginPlay() override;

	virtual void Logout(AController* Exiting) override;

public:
	UFUNCTION(BlueprintCallable)
	void KickPlayer(const FSessionPlayerInfo& SessionInfo, const FText& KickReason);

	void SetPlayerInfo(const FSessionPlayerInfo& RequestInfo);

	void UpdatePlayers();

	bool IsAllPlayersReady() const;

protected:
	// 플레이어 컨트롤러를 생성하고, 플레이어 캐릭터를 스폰하는 함수
	virtual void SpawnPlayerCharacter(APlayerController* Controller);

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