#pragma once

#include "CoreMinimal.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "LCRoomGameMode.generated.h"

// HUD & EOS & RPC
UENUM(BlueprintType)
enum class EMapType : uint8
{
	LuinsMap	UMETA(DisplayName = "LuinsMap"),
	CaveMap		UMETA(DisplayName = "CaveMap"),
	ETC			UMETA(DisplayName = "ETC")
};

UCLASS()
class LASTCANARY_API ALCRoomGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALCRoomGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;


	UFUNCTION(BlueprintCallable, Category = "Instance")
	FORCEINLINE_DEBUGGABLE ULCGameInstance* GetLCGameInstance() const
	{
		return Cast<ULCGameInstance>(GetGameInstance());
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	TArray<TSoftObjectPtr<UWorld>> MapReferences;
	
	UFUNCTION(BlueprintCallable, Category = "Maps")
	void SelectGameMap(int32 MapIndex);
	
	void StartGame();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maps")
	EMapType SelectedMap;


};
