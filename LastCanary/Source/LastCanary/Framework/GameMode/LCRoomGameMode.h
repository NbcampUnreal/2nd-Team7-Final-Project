#pragma once

#include "CoreMinimal.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	TArray<TSoftObjectPtr<UWorld>> MapReferences;
	
	UFUNCTION(BlueprintCallable, Category = "Maps")
	void SelectGameMap(int32 MapIndex);
	
	void StartGame();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maps")
	EMapType SelectedMap;
	
};
